#include "detection/detection_system.h"

#include "movement_analysis/player_context.h"
#include "movement/movement.h"
#include "sdk/entity/ccsplayerpawn.h"

#include <algorithm>
#include <cmath>

CConVar<bool> cs2ac_triggerbot_debug("cs2ac_triggerbot_debug", FCVAR_NONE, "Show Triggerbot target entries, shot timing, evidence, and rejections",
									 false);

#define TRIGGERBOT_DEBUG(...) \
	do \
	{ \
		if (cs2ac_triggerbot_debug.GetBool()) \
			Msg("[CS2AC Triggerbot] " __VA_ARGS__); \
	} while (0)

namespace
{
	constexpr float playerHalfWidth = 16.0f;
	constexpr float maximumReactionMilliseconds = 60.0f;
	constexpr float maximumHeldAimDrift = 0.5f;
	constexpr int detectionThreshold = 5;
	constexpr int historyTicks = 16;
	constexpr int pendingLifetimeTicks = 2;
	constexpr size_t pendingLimit = 8;
	constexpr auto evidenceWindow = std::chrono::minutes(5);
	constexpr float bodyHeights[] = {8.0f, 46.0f, 64.0f};

	static_assert(ENGINE_FIXED_TICK_INTERVAL * 3.0f * 1000.0f < maximumReactionMilliseconds);
	static_assert(ENGINE_FIXED_TICK_INTERVAL * 4.0f * 1000.0f >= maximumReactionMilliseconds);
	static_assert(historyTicks == sizeof(std::uint16_t) * 8);

	struct ConeMatch
	{
		float distance {};
		float dot {-1.0f};
		bool inside {};
	};

	bool IsPlayingTeam(int team)
	{
		return team == CS_TEAM_T || team == CS_TEAM_CT;
	}

	void ClearRuntime(detection::TriggerbotPlayerData &data)
	{
		data.targets = {};
		data.pending.clear();
		data.heldAim = {};
		data.heldSinceTick = -1;
		data.lastServerTick = -1;
		data.team = CS_TEAM_NONE;
		data.hasHeldAim = false;
	}

	const detection::TriggerbotSample *FindSample(const detection::TriggerbotTargetData &target, int serverTick)
	{
		const auto &sample = target.samples[static_cast<unsigned int>(serverTick) % target.samples.size()];
		return sample.serverTick == serverTick ? &sample : nullptr;
	}

	ConeMatch MatchCone(const Vector &eye, const Vector &forward, const Vector &origin)
	{
		ConeMatch best;
		float bestMargin = -2.0f;
		for (float height : bodyHeights)
		{
			Vector direction = origin + Vector(0.0f, 0.0f, height) - eye;
			if (!detection::IsFinite(direction))
			{
				continue;
			}
			const float distance = direction.Length();
			if (!std::isfinite(distance) || distance < EPSILON)
			{
				continue;
			}
			direction /= distance;
			const float dot = std::clamp(DotProduct(forward, direction), -1.0f, 1.0f);
			const float minimumDot = distance / std::sqrt(distance * distance + playerHalfWidth * playerHalfWidth);
			const float margin = dot - minimumDot;
			if (margin > bestMargin)
			{
				bestMargin = margin;
				best.distance = distance;
				best.dot = dot;
			}
		}
		best.inside = bestMargin >= 0.0f;
		return best;
	}

	float ToDegrees(float dot)
	{
		return static_cast<float>(std::acos(std::clamp(dot, -1.0f, 1.0f)) * (180.0 / M_PI));
	}

	float ConeDegrees(float distance)
	{
		return static_cast<float>(std::atan2(playerHalfWidth, distance) * (180.0 / M_PI));
	}
} // namespace

namespace detection
{
	void TriggerbotModule::Load(AnnounceCallback announceCallback, ShotCorrelator *shotCorrelator)
	{
		announce = announceCallback;
		shots = shotCorrelator;
	}

	void TriggerbotModule::Unload()
	{
		Reset();
		shots = nullptr;
		announce = nullptr;
	}

	void TriggerbotModule::Reset()
	{
		playerData = {};
	}

	void TriggerbotModule::OnGameFrame(int currentTick)
	{
		if (!shots)
		{
			return;
		}
		const PositionFrame *frame = shots->FindFrame(currentTick);
		if (!frame)
		{
			return;
		}

		for (int observerIndex = 1; observerIndex <= MAXPLAYERS; ++observerIndex)
		{
			auto *observerPlayer = g_pCS2ACPlayerManager ? g_pCS2ACPlayerManager->ToPlayer(static_cast<u32>(observerIndex)) : nullptr;
			auto *observerPawn = observerPlayer ? observerPlayer->GetPlayerPawn() : nullptr;
			const auto &observer = frame->players[observerIndex];
			auto &data = playerData[observerIndex];
			if (!IsEligibleHuman(observerPlayer) || !observerPlayer->IsAlive() || !observerPawn || !observer.valid || !observer.alive
				|| observer.teleported || !IsPlayingTeam(observer.team) || !IsFinite(observer.eyePosition))
			{
				ClearRuntime(data);
				continue;
			}
			if (data.lastServerTick == currentTick)
			{
				continue;
			}
			if ((data.lastServerTick >= 0 && static_cast<std::int64_t>(currentTick) - data.lastServerTick != 1)
				|| (data.team && data.team != observer.team))
			{
				ClearRuntime(data);
			}
			data.lastServerTick = currentTick;
			data.team = observer.team;

			const int observerSlot = observerPlayer->GetPlayerSlot().Get();
			const QAngle view = observerPawn->m_angEyeAngles();
			if (observerSlot < 0 || observerSlot >= MAXPLAYERS || !IsFinite(view))
			{
				ClearRuntime(data);
				continue;
			}
			const Vector forward = AimForward(view);
			if (!data.hasHeldAim || AngularDistance(data.heldAim, view) > maximumHeldAimDrift)
			{
				data.heldAim = view;
				data.heldSinceTick = currentTick;
				data.hasHeldAim = true;
			}
			const bool heldAngle = static_cast<std::int64_t>(currentTick) - data.heldSinceTick >= historyTicks;

			// The direct player-pair scan is bounded by the server's 64-player limit.
			for (int targetIndex = 1; targetIndex <= MAXPLAYERS; ++targetIndex)
			{
				auto &targetData = data.targets[targetIndex];
				const TriggerbotSample *previous = FindSample(targetData, currentTick - 1);
				auto *targetPlayer = g_pCS2ACPlayerManager ? g_pCS2ACPlayerManager->ToPlayer(static_cast<u32>(targetIndex)) : nullptr;
				auto *targetPawn = targetPlayer ? targetPlayer->GetPlayerPawn() : nullptr;
				const auto &target = frame->players[targetIndex];
				const bool previousAlive = previous && previous->valid && previous->alive;
				if (targetIndex == observerIndex || !targetPawn || !target.valid || target.teleported || !IsPlayingTeam(target.team)
					|| target.team == observer.team || (!target.alive && !previousAlive) || !IsFinite(target.origin))
				{
					targetData = {};
					continue;
				}
				if (targetData.lastServerTick >= 0 && static_cast<std::int64_t>(currentTick) - targetData.lastServerTick != 1)
				{
					targetData = {};
					previous = nullptr;
				}

				const bool spotted = targetPawn->m_entitySpottedState().m_bSpottedByMask().IsBitSet(observerSlot);
				const bool newlyVisible = targetData.spottedSamples >= historyTicks && targetData.spottedHistory == 0 && spotted;
				targetData.spottedHistory = static_cast<std::uint16_t>((targetData.spottedHistory << 1) | static_cast<std::uint16_t>(spotted));
				targetData.spottedSamples = (std::min)(targetData.spottedSamples + 1, historyTicks);
				targetData.lastServerTick = currentTick;
				const bool visible = targetData.spottedHistory != 0;
				const ConeMatch cone = MatchCone(observer.eyePosition, forward, target.origin);
				const bool inside = target.alive && visible && cone.inside;
				int contactTick = -1;
				if (newlyVisible && inside && heldAngle)
				{
					contactTick = currentTick;
				}
				else if (inside && previous && previous->contactTick >= 0
						 && static_cast<std::int64_t>(currentTick) - previous->contactTick <= 5)
				{
					contactTick = previous->contactTick;
				}

				auto &sample = targetData.samples[static_cast<unsigned int>(currentTick) % targetData.samples.size()];
				sample = {currentTick, contactTick, cone.distance, cone.dot, true, target.alive, visible, inside};
				if (contactTick == currentTick)
				{
					TRIGGERBOT_DEBUG("%s held a %.3f degree angle as target %d became visible at tick %d: distance %.1f, error %.3f, cone %.3f.\n",
									 observerPlayer->GetName(), maximumHeldAimDrift, targetIndex, currentTick, cone.distance,
									 ToDegrees(cone.dot), ConeDegrees(cone.distance));
				}
			}

			for (auto pending = data.pending.begin(); pending != data.pending.end();)
			{
				if ((!pending->evaluated && EvaluateShot(observerPlayer, data, *pending) && !pending->eligible)
					|| static_cast<std::int64_t>(currentTick) - pending->serverTick > pendingLifetimeTicks)
				{
					pending = data.pending.erase(pending);
				}
				else
				{
					++pending;
				}
			}
		}
	}

	void TriggerbotModule::OnWeaponFire(MovementPlayer *player, ShotRecord &shot)
	{
		if (!IsEligibleHuman(player) || shot.playerIndex != player->index || shot.triggerbotConsumed || !shot.hasVisibleAngles
			|| !IsFinite(shot.visibleAngles) || !IsFinite(shot.eyePosition) || shot.serverTick < 0)
		{
			return;
		}
		TriggerbotPendingShot pending {shot.id, shot.serverTick, shot.visibleAngles, shot.eyePosition};
		auto &data = playerData[player->index];
		if (EvaluateShot(player, data, pending) && !pending.eligible)
		{
			shot.triggerbotConsumed = true;
			return;
		}
		data.pending.push_back(std::move(pending));
		while (data.pending.size() > pendingLimit)
		{
			data.pending.pop_front();
		}
	}

	bool TriggerbotModule::EvaluateShot(MovementPlayer *player, TriggerbotPlayerData &data, TriggerbotPendingShot &shot)
	{
		if (!shots)
		{
			shot.evaluated = true;
			return true;
		}
		const PositionFrame *frame = shots->FindFrame(shot.serverTick);
		if (!frame)
		{
			return false;
		}
		shot.evaluated = true;
		const auto &observer = frame->players[player->index];
		if (!observer.valid || !observer.alive || observer.teleported || !IsPlayingTeam(observer.team))
		{
			return true;
		}

		const Vector forward = AimForward(shot.visibleAngles);
		int matchedTarget = -1;
		int contactTick = -1;
		ConeMatch matchedCone;
		for (int targetIndex = 1; targetIndex <= MAXPLAYERS; ++targetIndex)
		{
			const auto &target = frame->players[targetIndex];
			const auto &targetData = data.targets[targetIndex];
			const TriggerbotSample *current = FindSample(targetData, shot.serverTick);
			const TriggerbotSample *previous = FindSample(targetData, shot.serverTick - 1);
			if (targetIndex == player->index || !current || !current->valid || !current->inside || current->contactTick < 0 || !previous
				|| !previous->valid || !previous->alive
				|| !target.valid || target.teleported || !IsPlayingTeam(target.team) || target.team == observer.team || !IsFinite(target.origin))
			{
				continue;
			}

			const ConeMatch cone = MatchCone(shot.eyePosition, forward, target.origin);
			if (!cone.inside)
			{
				continue;
			}
			if (matchedTarget != -1)
			{
				TRIGGERBOT_DEBUG("%s shot %llu rejected because targets %d and %d both matched.\n", player->GetName(),
								 static_cast<unsigned long long>(shot.shotId), matchedTarget, targetIndex);
				return true;
			}
			matchedTarget = targetIndex;
			matchedCone = cone;
			contactTick = current->contactTick;
		}

		if (matchedTarget == -1)
		{
			TRIGGERBOT_DEBUG("%s shot %llu rejected because it did not follow a fresh target appearance on a held angle.\n", player->GetName(),
							 static_cast<unsigned long long>(shot.shotId));
			return true;
		}
		const std::int64_t reactionTicks = static_cast<std::int64_t>(shot.serverTick) - contactTick;
		const float reactionMilliseconds = static_cast<float>(reactionTicks) * ENGINE_FIXED_TICK_INTERVAL * 1000.0f;
		if (reactionTicks < 0 || reactionMilliseconds >= maximumReactionMilliseconds)
		{
			TRIGGERBOT_DEBUG("%s shot %llu rejected on target %d: %.3f degree error inside a %.3f degree cone after %.3f ms.\n", player->GetName(),
							 static_cast<unsigned long long>(shot.shotId), matchedTarget, ToDegrees(matchedCone.dot),
							 ConeDegrees(matchedCone.distance), reactionMilliseconds);
			return true;
		}

		shot.targetIndex = matchedTarget;
		shot.reactionMilliseconds = reactionMilliseconds;
		shot.aimError = ToDegrees(matchedCone.dot);
		shot.eligible = true;
		TRIGGERBOT_DEBUG("%s shot %llu is waiting for damage on target %d: %.3f degree error, %.3f degree cone, %.3f ms.\n",
						 player->GetName(), static_cast<unsigned long long>(shot.shotId), matchedTarget, shot.aimError,
						 ConeDegrees(matchedCone.distance), reactionMilliseconds);
		return true;
	}

	void TriggerbotModule::OnPlayerHurt(MovementPlayer *attacker, MovementPlayer *victim, ShotRecord &shot)
	{
		if (!IsEligibleHuman(attacker) || !victim || attacker == victim || shot.playerIndex != attacker->index || shot.triggerbotConsumed)
		{
			return;
		}
		shot.triggerbotConsumed = true;
		auto &data = playerData[attacker->index];
		auto pending = std::find_if(data.pending.begin(), data.pending.end(),
									[&](const TriggerbotPendingShot &candidate)
									{
										return candidate.shotId == shot.id;
									});
		if (pending == data.pending.end())
		{
			TRIGGERBOT_DEBUG("%s damaging shot %llu rejected because it had no held-angle candidate.\n", attacker->GetName(),
							 static_cast<unsigned long long>(shot.id));
			return;
		}
		if (!pending->evaluated && !EvaluateShot(attacker, data, *pending))
		{
			TRIGGERBOT_DEBUG("%s damaging shot %llu rejected because its historical frame was unavailable.\n", attacker->GetName(),
							 static_cast<unsigned long long>(shot.id));
			data.pending.erase(pending);
			return;
		}
		const TriggerbotPendingShot candidate = *pending;
		data.pending.erase(pending);
		if (!candidate.eligible || candidate.targetIndex != victim->index)
		{
			TRIGGERBOT_DEBUG("%s damaging shot %llu rejected because target %d did not match hurt victim %d.\n", attacker->GetName(),
							 static_cast<unsigned long long>(shot.id), candidate.targetIndex, victim->index);
			return;
		}

		auto &evidence = data.evidence;
		const auto now = Clock::now();
		while (!evidence.empty() && now - evidence.front() >= evidenceWindow)
		{
			evidence.pop_front();
		}
		evidence.push_back(now);
		TRIGGERBOT_DEBUG("%s damaging shot %llu added evidence %d/%d on target %d: %.3f degree error, %.3f ms.\n", attacker->GetName(),
						 static_cast<unsigned long long>(shot.id), static_cast<int>(evidence.size()), detectionThreshold,
						 candidate.targetIndex, candidate.aimError, candidate.reactionMilliseconds);
		if (evidence.size() >= detectionThreshold)
		{
			if (announce)
			{
				announce("TRIGGERBOT", attacker,
						 tfm::format("%zu damaging held-angle shots followed fresh target appearances; latest response %.3f ms with %.3f degree aim error.",
									 evidence.size(), candidate.reactionMilliseconds, candidate.aimError));
			}
			evidence.clear();
		}
	}

	void TriggerbotModule::OnClientDisconnect(MovementPlayer *player)
	{
		if (!player || player->index < 1 || player->index > MAXPLAYERS)
		{
			return;
		}
		playerData[player->index] = {};
		for (auto &data : playerData)
		{
			data.targets[player->index] = {};
		}
	}
} // namespace detection

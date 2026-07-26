#include "detection/detection_system.h"

#include "movement_analysis/player_context.h"
#include "movement/movement.h"
#include "sdk/usercmd.h"

#include <algorithm>
#include <cmath>

CConVar<bool> cs2ac_aimlock_debug("cs2ac_aimlock_debug", FCVAR_NONE, "Show Aimlock tracking episodes and evidence", false);

#define AIMLOCK_DEBUG(...) \
	do \
	{ \
		if (cs2ac_aimlock_debug.GetBool()) \
			Msg("[CS2AC Aimlock] " __VA_ARGS__); \
	} while (0)

namespace
{
	constexpr size_t commandHistorySize = 96;
	constexpr int trackingTicks = static_cast<int>(ENGINE_FIXED_TICK_RATE * 2.0f);
	constexpr int rearmTicks = static_cast<int>(ENGINE_FIXED_TICK_RATE * 0.5f);
	constexpr float maximumError = 0.5f;
	constexpr float minimumDistance = 300.0f;
	constexpr float minimumTargetTravel = 20.0f;
	constexpr float minimumTargetRate = 10.0f;
	constexpr float minimumFollowFraction = 0.9f;
	constexpr float meaningfulMovement = 0.01f;
	constexpr int detectionThreshold = 2;
	constexpr auto evidenceWindow = std::chrono::minutes(10);
	constexpr float bodyHeights[] = {8.0f, 46.0f, 64.0f};

	constexpr bool IsContinuousSequence(std::int64_t commandDelta, std::int64_t clientTickDelta, std::int64_t serverTickDelta)
	{
		return commandDelta >= 0 && clientTickDelta >= 0 && serverTickDelta >= 1 && serverTickDelta <= 2;
	}

	static_assert(IsContinuousSequence(0, 0, 1));
	static_assert(IsContinuousSequence(2, 2, 1));
	static_assert(IsContinuousSequence(2, 2, 2));
	static_assert(!IsContinuousSequence(-1, 1, 1));
	static_assert(!IsContinuousSequence(1, -1, 1));
	static_assert(!IsContinuousSequence(1, 1, 3));

	struct Candidate
	{
		QAngle bearing;
		float error {180.0f};
		int targetIndex {-1};
		int bodyPoint {-1};
		bool valid {};
	};

	QAngle Bearing(const Vector &eye, const Vector &target)
	{
		const Vector delta = target - eye;
		constexpr float radiansToDegrees = static_cast<float>(180.0 / M_PI);
		const float horizontal = std::sqrt(delta.x * delta.x + delta.y * delta.y);
		return {-std::atan2(delta.z, horizontal) * radiansToDegrees, std::atan2(delta.y, delta.x) * radiansToDegrees, 0.0f};
	}

	void ClearTrack(detection::AimlockPlayerData &data)
	{
		data.track = {};
	}

	void ClearRuntime(detection::AimlockPlayerData &data)
	{
		data.commands.clear();
		data.pending = {};
		data.track = {};
		data.latchedTarget = -1;
		data.breakStartTick = -1;
		data.latched = false;
	}

	Candidate FindCandidate(const detection::PositionFrame &frame, int observerIndex, const Vector &eye, const QAngle &view)
	{
		Candidate best;
		if (observerIndex < 1 || observerIndex > MAXPLAYERS || !frame.players[observerIndex].valid || !frame.players[observerIndex].alive
			|| frame.players[observerIndex].teleported)
		{
			return best;
		}
		const int observerTeam = frame.players[observerIndex].team;
		const Vector forward = detection::AimForward(view);
		const float minimumDot = std::cos(maximumError * static_cast<float>(M_PI / 180.0));
		int matches = 0;

		// The direct player scan is bounded by the server's 64-player limit.
		for (int targetIndex = 1; targetIndex <= MAXPLAYERS; ++targetIndex)
		{
			const auto &target = frame.players[targetIndex];
			if (targetIndex == observerIndex || !target.valid || !target.alive || target.teleported || target.team == observerTeam)
			{
				continue;
			}
			if ((target.origin - eye).Length() < minimumDistance)
			{
				continue;
			}

			float targetBestDot = -1.0f;
			int targetBestBodyPoint = -1;
			Vector targetBestPoint;
			for (int bodyPoint = 0; bodyPoint < static_cast<int>(std::size(bodyHeights)); ++bodyPoint)
			{
				const Vector targetPoint = target.origin + Vector(0.0f, 0.0f, bodyHeights[bodyPoint]);
				Vector direction = targetPoint - eye;
				if (!detection::IsFinite(direction) || direction.LengthSqr() < EPSILON)
				{
					continue;
				}
				direction.NormalizeInPlace();
				const float dot = std::clamp(DotProduct(forward, direction), -1.0f, 1.0f);
				if (dot > targetBestDot)
				{
					targetBestDot = dot;
					targetBestBodyPoint = bodyPoint;
					targetBestPoint = targetPoint;
				}
			}
			if (targetBestDot < minimumDot)
			{
				continue;
			}

			++matches;
			Candidate candidate {
				Bearing(eye, targetBestPoint), static_cast<float>(std::acos(targetBestDot) * (180.0 / M_PI)), targetIndex, targetBestBodyPoint, true,
			};
			if (candidate.error < best.error)
			{
				best = candidate;
			}
		}

		best.valid = matches == 1 && detection::IsFinite(best.bearing) && std::isfinite(best.error);
		return best;
	}
} // namespace

namespace detection
{
	void AimlockModule::Load(AnnounceCallback announceCallback, ShotCorrelator *shotCorrelator)
	{
		announce = announceCallback;
		shots = shotCorrelator;
	}

	void AimlockModule::Unload()
	{
		Reset();
		shots = nullptr;
		announce = nullptr;
	}

	void AimlockModule::Reset()
	{
		playerData = {};
		evidence = {};
	}

	void AimlockModule::OnProcessUsercmds(MovementPlayer *player, PlayerCommand *commands, int numCommands)
	{
		if (!IsEligibleHuman(player) || !commands || numCommands <= 0)
		{
			return;
		}

		auto &data = playerData[player->index];
		for (int i = 0; i < numCommands; ++i)
		{
			PlayerCommand &command = commands[i];
			if (!command.has_base() || !command.base().has_viewangles())
			{
				continue;
			}
			if (std::any_of(data.commands.rbegin(), data.commands.rend(),
							[&](const AimlockCommand &stored) { return stored.commandNumber == command.cmdNum; }))
			{
				continue;
			}
			const auto &base = command.base();
			const QAngle angles(base.viewangles().x(), base.viewangles().y(), base.viewangles().z());
			if (!IsFinite(angles))
			{
				continue;
			}
			data.commands.push_back({command.cmdNum, base.client_tick(), angles});
			while (data.commands.size() > commandHistorySize)
			{
				data.commands.pop_front();
			}
		}
	}

	void AimlockModule::OnSetupMove(MovementPlayer *player, PlayerCommand *command, int currentTick)
	{
		if (!IsEligibleHuman(player) || !command)
		{
			return;
		}
		auto &data = playerData[player->index];
		auto found = std::find_if(data.commands.rbegin(), data.commands.rend(),
								  [&](const AimlockCommand &stored) { return stored.commandNumber == command->cmdNum; });
		if (found == data.commands.rend())
		{
			ClearTrack(data);
			data.pending = {};
			return;
		}
		Vector eye;
		player->GetEyeOrigin(&eye);
		if (!IsFinite(eye))
		{
			ClearTrack(data);
			data.pending = {};
			return;
		}
		data.pending = {found->commandNumber, found->clientTick, currentTick, found->angles, eye, true};
	}

	void AimlockModule::OnGameFrame(int currentTick)
	{
		if (!shots)
		{
			return;
		}
		for (int index = 1; index <= MAXPLAYERS; ++index)
		{
			auto &data = playerData[index];
			auto *player = g_pCS2ACPlayerManager ? g_pCS2ACPlayerManager->ToPlayer(static_cast<u32>(index)) : nullptr;
			if (!IsEligibleHuman(player) || !player->IsAlive())
			{
				ClearRuntime(data);
				continue;
			}
			if (!data.pending.valid)
			{
				continue;
			}

			const AimlockSample sample = data.pending;
			data.pending = {};
			if (sample.serverTick != currentTick || !shots->FindFrame(sample.serverTick))
			{
				AIMLOCK_DEBUG("%s tracking reset because the command and world snapshot did not share a tick.\n", player->GetName());
				ClearTrack(data);
				continue;
			}
			Evaluate(player, data, sample);
		}
	}

	void AimlockModule::Evaluate(MovementPlayer *player, AimlockPlayerData &data, const AimlockSample &sample)
	{
		const PositionFrame *frame = shots ? shots->FindFrame(sample.serverTick) : nullptr;
		if (!frame || !IsFinite(sample.angles))
		{
			ClearTrack(data);
			return;
		}
		const auto &observer = frame->players[player->index];
		if (!observer.valid || !IsFinite(observer.eyePosition))
		{
			ClearTrack(data);
			return;
		}
		const Candidate candidate = FindCandidate(*frame, player->index, observer.eyePosition, sample.angles);

		if (data.latched)
		{
			if (candidate.valid && candidate.targetIndex == data.latchedTarget)
			{
				data.breakStartTick = -1;
				return;
			}
			if (data.breakStartTick < 0)
			{
				data.breakStartTick = sample.serverTick;
			}
			if (static_cast<std::int64_t>(sample.serverTick) - data.breakStartTick < rearmTicks)
			{
				return;
			}
			data.latched = false;
			data.latchedTarget = -1;
			data.breakStartTick = -1;
			AIMLOCK_DEBUG("%s rearmed after a half-second break.\n", player->GetName());
		}

		auto startTrack = [&]()
		{
			if (!candidate.valid)
			{
				return;
			}
			data.track = {};
			data.track.targetIndex = candidate.targetIndex;
			data.track.bodyPoint = candidate.bodyPoint;
			data.track.startServerTick = sample.serverTick;
			data.track.lastServerTick = sample.serverTick;
			data.track.lastClientTick = sample.clientTick;
			data.track.lastCommandNumber = sample.commandNumber;
			data.track.lastView = sample.angles;
			data.track.lastBearing = candidate.bearing;
			AIMLOCK_DEBUG("%s locked target %d body point %d at %.3f degrees.\n", player->GetName(), candidate.targetIndex, candidate.bodyPoint,
						  candidate.error);
		};

		if (data.track.targetIndex < 0)
		{
			startTrack();
			return;
		}
		const std::int64_t commandDelta = static_cast<std::int64_t>(sample.commandNumber) - data.track.lastCommandNumber;
		const std::int64_t clientTickDelta = static_cast<std::int64_t>(sample.clientTick) - data.track.lastClientTick;
		const std::int64_t serverTickDelta = static_cast<std::int64_t>(sample.serverTick) - data.track.lastServerTick;
		if (serverTickDelta == 0 && commandDelta >= 0 && clientTickDelta >= 0)
		{
			return;
		}
		if (!candidate.valid && serverTickDelta == 1 && commandDelta >= 0 && clientTickDelta >= 0)
		{
			AIMLOCK_DEBUG("%s kept target %d through one missing target sample.\n", player->GetName(), data.track.targetIndex);
			return;
		}
		if (!candidate.valid || candidate.targetIndex != data.track.targetIndex
			|| !IsContinuousSequence(commandDelta, clientTickDelta, serverTickDelta))
		{
			AIMLOCK_DEBUG("%s tracking reset: target %d->%d, command delta %lld, client tick delta %lld, server tick delta %lld.\n",
						  player->GetName(), data.track.targetIndex, candidate.valid ? candidate.targetIndex : -1,
						  static_cast<long long>(commandDelta), static_cast<long long>(clientTickDelta), static_cast<long long>(serverTickDelta));
			ClearTrack(data);
			startTrack();
			return;
		}

		const bool bodyPointChanged = candidate.bodyPoint != data.track.bodyPoint;
		const Vector lastTarget = AimForward(data.track.lastBearing);
		const Vector target = AimForward(candidate.bearing);
		const Vector lastView = AimForward(data.track.lastView);
		const Vector view = AimForward(sample.angles);
		Vector targetDelta = target - lastTarget;
		Vector viewDelta = view - lastView;
		const float targetTravel = bodyPointChanged ? 0.0f : AngularDistance(data.track.lastBearing, candidate.bearing);
		const float viewTravel = bodyPointChanged ? 0.0f : AngularDistance(data.track.lastView, sample.angles);
		const bool moving = std::isfinite(targetTravel) && targetTravel >= meaningfulMovement;
		const bool viewMoving = std::isfinite(viewTravel) && viewTravel >= meaningfulMovement;
		float alignment = 0.0f;
		if (moving && viewMoving && targetDelta.LengthSqr() >= EPSILON && viewDelta.LengthSqr() >= EPSILON)
		{
			targetDelta.NormalizeInPlace();
			viewDelta.NormalizeInPlace();
			alignment = std::clamp(DotProduct(targetDelta, viewDelta), -1.0f, 1.0f);
		}
		const bool followed = moving && viewMoving && alignment > 0.0f;
		const float followedTravel = followed ? (std::min)(targetTravel, viewTravel) * std::clamp(alignment, 0.0f, 1.0f) : 0.0f;

		data.track.motions.push_back({sample.serverTick, targetTravel, followedTravel, followed, moving});
		data.track.targetTravel += targetTravel;
		data.track.followedTravel += followedTravel;
		data.track.movingSamples += moving;
		data.track.followedSamples += followed;
		while (!data.track.motions.empty()
			   && static_cast<std::int64_t>(data.track.motions.front().serverTick) <= static_cast<std::int64_t>(sample.serverTick) - trackingTicks)
		{
			const AimlockMotion expired = data.track.motions.front();
			data.track.motions.pop_front();
			data.track.targetTravel -= expired.targetTravel;
			data.track.followedTravel -= expired.followedTravel;
			data.track.movingSamples -= expired.moving;
			data.track.followedSamples -= expired.followed;
		}

		data.track.lastServerTick = sample.serverTick;
		data.track.lastClientTick = sample.clientTick;
		data.track.lastCommandNumber = sample.commandNumber;
		data.track.lastView = sample.angles;
		data.track.lastBearing = candidate.bearing;
		data.track.bodyPoint = candidate.bodyPoint;

		if (static_cast<std::int64_t>(sample.serverTick) - data.track.startServerTick < trackingTicks || data.track.targetTravel < minimumTargetTravel
			|| data.track.targetTravel / 2.0f < minimumTargetRate || data.track.movingSamples == 0
			|| static_cast<float>(data.track.followedSamples) / data.track.movingSamples < minimumFollowFraction
			|| data.track.followedTravel / data.track.targetTravel < minimumFollowFraction)
		{
			return;
		}
		AddIncident(player, data);
	}

	void AimlockModule::AddIncident(MovementPlayer *player, AimlockPlayerData &data)
	{
		const auto now = Clock::now();
		auto &incidents = evidence[player->index];
		while (!incidents.empty() && now - incidents.front() >= evidenceWindow)
		{
			incidents.pop_front();
		}
		incidents.push_back(now);
		AIMLOCK_DEBUG("%s added evidence %d/%d.\n", player->GetName(), static_cast<int>(incidents.size()), detectionThreshold);
		if (incidents.size() >= detectionThreshold)
		{
			if (announce)
			{
				announce("AIMLOCK", player,
						 tfm::format("%zu precise tracking episodes reached the threshold; latest episode followed %.1f%% of target travel.",
									 incidents.size(), data.track.targetTravel > 0.0f
														   ? data.track.followedTravel * 100.0f / data.track.targetTravel
														   : 0.0f));
			}
			incidents.clear();
		}

		data.latched = true;
		data.latchedTarget = data.track.targetIndex;
		data.breakStartTick = -1;
		ClearTrack(data);
	}

	void AimlockModule::OnClientDisconnect(MovementPlayer *player)
	{
		if (player && player->index >= 1 && player->index <= MAXPLAYERS)
		{
			playerData[player->index] = {};
			evidence[player->index].clear();
		}
	}
} // namespace detection

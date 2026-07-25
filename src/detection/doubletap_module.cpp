#include "detection/detection_system.h"

#include "movement/movement.h"

namespace
{
	constexpr int detectionThreshold = 2;
}

namespace detection
{
	void DoubletapModule::Load(AnnounceCallback announceCallback)
	{
		announce = announceCallback;
	}

	void DoubletapModule::Unload()
	{
		Reset();
		announce = nullptr;
	}

	void DoubletapModule::Reset()
	{
		playerData = {};
	}

	void DoubletapModule::OnWeaponFire(MovementPlayer *player, const ShotRecord &shot)
	{
		if (!IsEligibleHuman(player) || shot.playerIndex != player->index || !IsBallisticWeapon(shot.weapon))
		{
			return;
		}

		auto &previous = playerData[player->index];
		const std::int64_t delta = static_cast<std::int64_t>(shot.serverTick) - previous.serverTick;
		const bool detected = previous.shotId && previous.shotId != shot.id && NormalizeWeapon(previous.weapon) == NormalizeWeapon(shot.weapon)
							  && delta >= 0 && delta <= 1;
		if (detected)
		{
			const int incidents = previous.incidents + 1;
			previous = {};
			if (incidents >= detectionThreshold)
			{
				if (announce)
				{
					announce("DOUBLETAP", player,
							 tfm::format("%d rapid-fire pairs from %s reached the threshold; the latest pair was %d server tick%s apart.",
										 incidents, shot.weapon, delta, delta == 1 ? "" : "s"));
				}
			}
			else
			{
				previous.incidents = incidents;
			}
			return;
		}

		previous.shotId = shot.id;
		previous.serverTick = shot.serverTick;
		previous.weapon = shot.weapon;
	}

	void DoubletapModule::OnClientDisconnect(MovementPlayer *player)
	{
		if (player && player->index >= 1 && player->index <= MAXPLAYERS)
		{
			playerData[player->index] = {};
		}
	}
} // namespace detection

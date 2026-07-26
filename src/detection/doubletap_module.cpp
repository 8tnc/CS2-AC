#include "detection/detection_system.h"

#include "igameevents.h"
#include "movement/movement.h"

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

	void DoubletapModule::OnWeaponFire(IGameEvent *event, MovementPlayer *player, int currentTick)
	{
		if (!event || !IsEligibleHuman(player))
		{
			return;
		}

		const std::string_view weaponName = NormalizeWeapon(event->GetString("weapon", ""));
		if (!IsBallisticWeapon(weaponName) || weaponName == "revolver")
		{
			return;
		}

		auto &previous = playerData[player->index];
		auto *pawn = player->GetPlayerPawn();
		auto *weaponServices = pawn ? pawn->m_pWeaponServices() : nullptr;
		auto *activeWeapon = weaponServices ? weaponServices->m_hActiveWeapon().Get() : nullptr;
		const char *activeWeaponName = activeWeapon ? activeWeapon->GetClassname() : nullptr;
		if (!activeWeaponName || NormalizeWeapon(activeWeaponName) != weaponName)
		{
			previous = {};
			return;
		}
		auto *weaponData = activeWeapon ? activeWeapon->GetWeaponVData() : nullptr;
		if (!weaponData)
		{
			previous = {};
			return;
		}

		const float cycleTime = weaponData->m_flCycleTime().m_flValues[0];
		if (!std::isfinite(cycleTime) || cycleTime <= 0.0f)
		{
			previous = {};
			return;
		}

		const std::int64_t delta = static_cast<std::int64_t>(currentTick) - previous.serverTick;
		const bool detected = previous.serverTick >= 0 && delta >= 0 && NormalizeWeapon(previous.weapon) == weaponName
							  && static_cast<float>(delta) <= cycleTime * ENGINE_FIXED_TICK_RATE - 1.0f;
		previous.serverTick = currentTick;
		previous.weapon.assign(weaponName);

		if (detected)
		{
			previous = {};
			if (announce)
			{
				announce("DOUBLETAP", player,
						 tfm::format("%s fired twice only %d server tick%s apart, sooner than its %.3f-second cycle allows.", weaponName, delta,
									 delta == 1 ? "" : "s", cycleTime));
			}
		}
	}

	void DoubletapModule::OnClientDisconnect(MovementPlayer *player)
	{
		if (player && player->index >= 1 && player->index <= MAXPLAYERS)
		{
			playerData[player->index] = {};
		}
	}
} // namespace detection

#include "detection/detection_system.h"

#include "igameevents.h"
#include "movement/movement.h"

CConVar<bool> cs2ac_doubletap_debug("cs2ac_doubletap_debug", FCVAR_NONE, "Show Doubletap weapon-fire validation and tick spacing", false);

#define DOUBLETAP_DEBUG(...) \
	do \
	{ \
		if (cs2ac_doubletap_debug.GetBool()) \
			Msg("[CS2AC Doubletap] " __VA_ARGS__); \
	} while (0)

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
			DOUBLETAP_DEBUG("%s fire ignored: %.*s is not eligible.\n", player->GetName(), static_cast<int>(weaponName.size()), weaponName.data());
			return;
		}

		auto &previous = playerData[player->index];
		auto *pawn = player->GetPlayerPawn();
		auto *weaponServices = pawn ? pawn->m_pWeaponServices() : nullptr;
		auto *activeWeapon = weaponServices ? weaponServices->m_hActiveWeapon().Get() : nullptr;
		const char *activeWeaponName = activeWeapon ? activeWeapon->GetClassname() : nullptr;
		if (!activeWeaponName || NormalizeWeapon(activeWeaponName) != weaponName)
		{
			DOUBLETAP_DEBUG("%s fire rejected: the active weapon does not match %.*s.\n", player->GetName(), static_cast<int>(weaponName.size()),
							weaponName.data());
			previous = {};
			return;
		}
		auto *weaponData = activeWeapon ? activeWeapon->GetWeaponVData() : nullptr;
		if (!weaponData)
		{
			DOUBLETAP_DEBUG("%s fire rejected: %.*s has no live weapon data.\n", player->GetName(), static_cast<int>(weaponName.size()),
							weaponName.data());
			previous = {};
			return;
		}

		const float cycleTime = weaponData->m_flCycleTime().m_flValues[0];
		if (!std::isfinite(cycleTime) || cycleTime <= 0.0f)
		{
			DOUBLETAP_DEBUG("%s fire rejected: %.*s returned invalid cycle time %.6f.\n", player->GetName(), static_cast<int>(weaponName.size()),
							weaponName.data(), cycleTime);
			previous = {};
			return;
		}

		const std::int64_t delta = static_cast<std::int64_t>(currentTick) - previous.serverTick;
		const bool detected = previous.serverTick >= 0 && delta >= 0 && NormalizeWeapon(previous.weapon) == weaponName
							  && static_cast<float>(delta) <= cycleTime * ENGINE_FIXED_TICK_RATE - 1.0f;
		if (previous.serverTick < 0)
		{
			DOUBLETAP_DEBUG("%s stored the first %.*s fire at server tick %d.\n", player->GetName(), static_cast<int>(weaponName.size()),
							weaponName.data(), currentTick);
		}
		else
		{
			DOUBLETAP_DEBUG("%s fired %.*s %lld tick%s after the previous %s fire; maximum suspicious spacing is %.2f ticks. %s\n", player->GetName(),
							static_cast<int>(weaponName.size()), weaponName.data(), static_cast<long long>(delta), delta == 1 ? "" : "s",
							previous.weapon.c_str(), cycleTime * ENGINE_FIXED_TICK_RATE - 1.0f, detected ? "Matched." : "Rejected.");
		}
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

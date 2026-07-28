#include "detection/detection_system.h"

#include "igameevents.h"
#include "movement/movement.h"

CConVar<bool> cs2ac_doubletap_debug("cs2ac_doubletap_debug", FCVAR_NONE, "Show Doubletap weapon-fire tick spacing", false);

namespace
{
	constexpr int detectionThreshold = 3;
}

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

		auto &previous = playerData[player->index];
		const std::string_view weapon = NormalizeWeapon(event->GetString("weapon", ""));
		if (!IsBallisticWeapon(weapon))
		{
			previous.serverTick = -1;
			previous.weapon.clear();
			return;
		}
		if (previous.serverTick < 0)
		{
			DOUBLETAP_DEBUG("%s stored the first %.*s fire at server tick %d.\n", player->GetName(), static_cast<int>(weapon.size()), weapon.data(),
							currentTick);
			previous.serverTick = currentTick;
			previous.weapon.assign(weapon);
			return;
		}

		const std::int64_t delta = static_cast<std::int64_t>(currentTick) - previous.serverTick;
		if (delta < 0 || delta > 1 || previous.weapon != weapon)
		{
			DOUBLETAP_DEBUG("%s fired %.*s %lld server ticks after %s. Rejected.\n", player->GetName(), static_cast<int>(weapon.size()),
							weapon.data(), static_cast<long long>(delta), previous.weapon.c_str());
			previous.serverTick = currentTick;
			previous.weapon.assign(weapon);
			return;
		}

		previous.serverTick = currentTick;
		const int incidents = ++previous.incidents;
		DOUBLETAP_DEBUG("%s matched pair %d/%d at %lld server tick%s apart.\n", player->GetName(), incidents, detectionThreshold,
						static_cast<long long>(delta), delta == 1 ? "" : "s");
		if (incidents < detectionThreshold)
		{
			return;
		}
		previous.incidents = 0;

		if (announce)
		{
			announce("DOUBLETAP", player,
					 localization::Format(delta == 1 ? "evidence.doubletap.one_tick" : "evidence.doubletap.zero_ticks",
										  delta == 1 ? "Three rapid-fire pairs reached the threshold; the latest pair was 1 server tick apart."
													 : "Three rapid-fire pairs reached the threshold; the latest pair was 0 server ticks apart."));
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

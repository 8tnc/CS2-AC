#include "detection/detection_system.h"

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

	void DoubletapModule::OnWeaponFire(MovementPlayer *player, int currentTick)
	{
		if (!IsEligibleHuman(player))
		{
			return;
		}

		auto &previous = playerData[player->index];
		if (previous.serverTick < 0)
		{
			DOUBLETAP_DEBUG("%s stored the first fire at server tick %d.\n", player->GetName(), currentTick);
			previous.serverTick = currentTick;
			return;
		}

		const std::int64_t delta = static_cast<std::int64_t>(currentTick) - previous.serverTick;
		if (delta > 1)
		{
			DOUBLETAP_DEBUG("%s fired %lld server ticks after the previous fire. Rejected.\n", player->GetName(), static_cast<long long>(delta));
			previous.serverTick = currentTick;
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
					 tfm::format("Three rapid-fire pairs reached the threshold; the latest pair was %d server tick%s apart.", static_cast<int>(delta),
								 delta == 1 ? "" : "s"));
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

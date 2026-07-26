#include "detection/detection_system.h"

#include "movement/movement.h"
#include "movement_analysis/player_context.h"

namespace
{
	constexpr size_t detectionThreshold = 5;
	constexpr auto evidenceWindow = std::chrono::seconds(60);
} // namespace

namespace detection
{
	void NameChangerModule::Load(AnnounceCallback announceCallback)
	{
		announce = announceCallback;
		Reset();
	}

	void NameChangerModule::Unload()
	{
		playerData = {};
		announce = nullptr;
	}

	void NameChangerModule::Reset()
	{
		playerData = {};
		if (!g_pCS2ACPlayerManager)
		{
			return;
		}
		for (int index = 1; index <= MAXPLAYERS; ++index)
		{
			OnClientReady(g_pCS2ACPlayerManager->ToPlayer(static_cast<u32>(index)));
		}
	}

	void NameChangerModule::OnClientReady(MovementPlayer *player)
	{
		if (!IsEligibleHuman(player) || !player->IsInGame())
		{
			return;
		}
		auto &data = playerData[player->index];
		data = {};
		const char *name = player->GetClient()->GetClientName();
		if (name)
		{
			data.lastName = name;
			data.initialized = true;
		}
	}

	void NameChangerModule::OnClientSettingsChanged(MovementPlayer *player)
	{
		if (!IsEligibleHuman(player) || !player->IsInGame())
		{
			return;
		}
		const char *currentName = player->GetClient()->GetClientName();
		if (!currentName)
		{
			return;
		}

		auto &data = playerData[player->index];
		if (!data.initialized)
		{
			data.lastName = currentName;
			data.initialized = true;
			return;
		}
		if (data.lastName == currentName)
		{
			return;
		}
		data.lastName = currentName;

		const auto now = Clock::now();
		while (!data.changes.empty() && now - data.changes.front() >= evidenceWindow)
		{
			data.changes.pop_front();
		}
		data.changes.push_back(now);
		if (data.changes.size() >= detectionThreshold)
		{
			if (announce)
			{
				announce("NAMECHANGER", player, tfm::format("%zu visible name changes occurred within one minute.", data.changes.size()));
			}
			data.changes.clear();
		}
	}

	void NameChangerModule::OnClientDisconnect(MovementPlayer *player)
	{
		if (player && player->index >= 1 && player->index <= MAXPLAYERS)
		{
			playerData[player->index] = {};
		}
	}
} // namespace detection

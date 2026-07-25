#include "player_context.h"

#include "tier0/memdbgon.h"
CS2ACPlayerManager g_CS2ACPlayerManager;

CS2ACPlayerManager *g_pCS2ACPlayerManager = &g_CS2ACPlayerManager;
PlayerManager *g_pPlayerManager = dynamic_cast<PlayerManager *>(&g_CS2ACPlayerManager);

CS2ACPlayerManager::CS2ACPlayerManager()
{
	for (i32 i = 0; i <= MAXPLAYERS; ++i)
	{
		delete players[i];
		players[i] = new CS2ACPlayer(i);
	}
}

CS2ACPlayer *CS2ACPlayerManager::ToPlayer(CPlayerPawnComponent *component)
{
	return static_cast<CS2ACPlayer *>(MovementPlayerManager::ToPlayer(component));
}

CS2ACPlayer *CS2ACPlayerManager::ToPlayer(CBasePlayerController *controller)
{
	return static_cast<CS2ACPlayer *>(MovementPlayerManager::ToPlayer(controller));
}

CS2ACPlayer *CS2ACPlayerManager::ToPlayer(CBasePlayerPawn *pawn)
{
	return static_cast<CS2ACPlayer *>(MovementPlayerManager::ToPlayer(pawn));
}

CS2ACPlayer *CS2ACPlayerManager::ToPlayer(CPlayerSlot slot)
{
	return static_cast<CS2ACPlayer *>(MovementPlayerManager::ToPlayer(slot));
}

CS2ACPlayer *CS2ACPlayerManager::ToPlayer(CEntityIndex entIndex)
{
	return static_cast<CS2ACPlayer *>(MovementPlayerManager::ToPlayer(entIndex));
}

CS2ACPlayer *CS2ACPlayerManager::ToPlayer(CPlayerUserId userID)
{
	return static_cast<CS2ACPlayer *>(MovementPlayerManager::ToPlayer(userID));
}

CS2ACPlayer *CS2ACPlayerManager::ToPlayer(u32 index)
{
	return static_cast<CS2ACPlayer *>(MovementPlayerManager::players[index]);
}

CS2ACPlayer *CS2ACPlayerManager::SteamIdToPlayer(u64 steamID, bool validated)
{
	return static_cast<CS2ACPlayer *>(PlayerManager::SteamIdToPlayer(steamID, validated));
}

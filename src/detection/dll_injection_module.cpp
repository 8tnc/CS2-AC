#include "detection/detection_system.h"

#include "igameevents.h"
#include "movement_analysis/player_context.h"
#include "utils/interfaces.h"
#include "utils/utils.h"

namespace
{
	constexpr auto scanInterval = std::chrono::seconds(10);

	// This conservative list excludes client_disconnect and local_player_team because normal clients use them.
	constexpr std::array<const char *, 136> blacklistedEvents = {
		"gameui_hidden",
		"player_chat",
		"player_score",
		"player_shoot",
		"game_init",
		"game_start",
		"game_end",
		"ugc_map_info_received",
		"ugc_map_unsubscribed",
		"ugc_map_download_error",
		"ugc_file_download_finished",
		"ugc_file_download_start",
		"dm_bonus_weapon_start",
		"survival_announce_phase",
		"entity_visible",
		"instructor_server_hint_create",
		"instructor_server_hint_stop",
		"reset_game_titledata",
		"vote_ended",
		"vote_started",
		"vote_options",
		"endmatch_mapvote_selecting_map",
		"endmatch_cmm_start_reveal_items",
		"inventory_updated",
		"client_loadout_changed",
		"add_player_sonar_icon",
		"door_open",
		"door_closed",
		"door_break",
		"bullet_damage",
		"bomb_abortplant",
		"hostage_stops_following",
		"hostage_call_for_help",
		"vip_escaped",
		"vip_killed",
		"silencer_detach",
		"inspect_weapon",
		"weapon_zoom_rifle",
		"player_spawned",
		"item_pickup_slerp",
		"item_pickup_failed",
		"item_equip",
		"enter_buyzone",
		"exit_buyzone",
		"buytime_ended",
		"enter_bombzone",
		"exit_bombzone",
		"enter_rescue_zone",
		"exit_rescue_zone",
		"silencer_off",
		"silencer_on",
		"buymenu_open",
		"buymenu_close",
		"round_poststart",
		"tagrenade_detonate",
		"inferno_extinguish",
		"player_jump",
		"mb_input_lock_success",
		"mb_input_lock_cancel",
		"nav_generate",
		"achievement_info_loaded",
		"hltv_changed_mode",
		"show_deathpanel",
		"hide_deathpanel",
		"player_avenged_teammate",
		"achievement_earned_local",
		"repost_xbox_achievements",
		"match_end_conditions",
		"write_profile_data",
		"trial_time_expired",
		"update_matchmaking_stats",
		"enable_restart_voting",
		"sfuievent",
		"start_vote",
		"teamchange_pending",
		"material_default_complete",
		"cs_prev_next_spectator",
		"tournament_reward",
		"start_halftime",
		"ammo_refill",
		"parachute_pickup",
		"parachute_deploy",
		"dronegun_attack",
		"drone_dispatched",
		"loot_crate_visible",
		"loot_crate_opened",
		"open_crate_instr",
		"smoke_beacon_paradrop",
		"drone_cargo_detached",
		"drone_above_roof",
		"dz_item_interaction",
		"survival_teammate_respawn",
		"guardian_wave_restart",
		"bullet_flight_resolution",
		"server_message",
		"player_full_update",
		"local_player_controller_team",
		"local_player_pawn_changed",
		"ragdoll_dissolved",
		"team_info",
		"team_score",
		"hltv_rank_camera",
		"hltv_rank_entity",
		"demo_stop",
		"map_shutdown",
		"map_transition",
		"hostname_changed",
		"game_message",
		"round_start_pre_entity",
		"round_start_post_nav",
		"player_hintmessage",
		"broken_breakable",
		"door_close",
		"vote_failed",
		"vote_passed",
		"vote_cast_yes",
		"vote_cast_no",
		"achievement_event",
		"achievement_write_failed",
		"bonus_updated",
		"gameinstructor_draw",
		"gameinstructor_nodraw",
		"flare_ignite_npc",
		"helicopter_grenade_punt_miss",
		"physgun_pickup",
		"cart_updated",
		"store_pricesheet_updated",
		"item_schema_initialized",
		"drop_rate_modified",
		"event_ticket_modified",
		"gc_connected",
		"instructor_start_lesson",
		"instructor_close_lesson",
		"set_instructor_group_enabled",
		"clientside_lesson_closed",
		"dynamic_shadow_light_changed",
	};

	static_assert(blacklistedEvents.size() == 136, "The DLL Injection event list must contain 136 entries.");
} // namespace

namespace detection
{
	void DllInjectionModule::Load(AnnounceCallback callback)
	{
		announce = callback;
		Reset();
	}

	void DllInjectionModule::Unload()
	{
		activeEvents = {};
		announce = nullptr;
	}

	void DllInjectionModule::Reset()
	{
		activeEvents = {};
		nextScan = std::chrono::steady_clock::now() + scanInterval;
	}

	void DllInjectionModule::OnGameFrame()
	{
		auto now = std::chrono::steady_clock::now();
		if (now < nextScan)
		{
			return;
		}
		nextScan = now + scanInterval;

		if (!announce || !interfaces::pGameEventManager || !g_pCS2ACPlayerManager || !g_pCS2ACUtils)
		{
			return;
		}

		for (u32 index = 1; index <= MAXPLAYERS; ++index)
		{
			auto *player = g_pCS2ACPlayerManager->ToPlayer(index);
			if (!player || !player->IsConnected() || !player->IsInGame() || player->IsFakeClient() || player->IsCSTV())
			{
				continue;
			}

			const int slot = player->GetPlayerSlot().Get();
			if (slot < 0 || slot >= MAXPLAYERS)
			{
				continue;
			}

			auto *listener = g_pCS2ACUtils->GetLegacyGameEventListener(player->GetPlayerSlot());
			if (!listener)
			{
				continue;
			}

			std::bitset<136> current;
			for (size_t eventIndex = 0; eventIndex < blacklistedEvents.size(); ++eventIndex)
			{
				if (!interfaces::pGameEventManager->FindListener(listener, blacklistedEvents[eventIndex]))
				{
					continue;
				}

				current.set(eventIndex);
			}
			const bool detected = current.any();
			activeEvents[slot] = current;

			if (!detected)
			{
				continue;
			}

			std::string matches;
			for (size_t eventIndex = 0; eventIndex < blacklistedEvents.size(); ++eventIndex)
			{
				if (!current.test(eventIndex))
				{
					continue;
				}
				if (!matches.empty())
				{
					matches += ", ";
				}
				matches += blacklistedEvents[eventIndex];
				if (matches.size() > 700)
				{
					matches += ", ...";
					break;
				}
			}
			announce("DLL INJECTION", player,
					 tfm::format("%zu blacklisted client event subscription%s found: %s.", current.count(), current.count() == 1 ? "" : "s",
								 matches));
		}
	}

	void DllInjectionModule::OnClientDisconnect(MovementPlayer *player)
	{
		if (!player)
		{
			return;
		}

		const int slot = player->GetPlayerSlot().Get();
		if (slot >= 0 && slot < MAXPLAYERS)
		{
			activeEvents[slot].reset();
		}
	}
} // namespace detection

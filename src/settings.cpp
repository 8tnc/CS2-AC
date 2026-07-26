#include "settings.h"

#include "convar.h"
#include "eiface.h"
#include "utils/interfaces.h"

#include <algorithm>
#include <charconv>
#include <new>
#include <sstream>
#include <string>
#include <vector>

namespace
{
	std::size_t rejectedWhitelistEntries {};
	std::size_t duplicateWhitelistEntries {};
	std::uint64_t settingsRevision {1};

	std::vector<std::uint64_t> &WhitelistedSteamIds()
	{
		static std::vector<std::uint64_t> steamIds;
		return steamIds;
	}

	void BumpRevision()
	{
		if (++settingsRevision == 0)
		{
			settingsRevision = 1;
		}
	}

	void OnWhitelistChanged(CConVar<CUtlString> *, CSplitScreenSlot, const CUtlString *newValue, const CUtlString *)
	{
		auto &steamIds = WhitelistedSteamIds();
		steamIds.clear();
		rejectedWhitelistEntries = 0;
		duplicateWhitelistEntries = 0;
		std::string value = newValue ? newValue->Get() : "";
		std::replace(value.begin(), value.end(), ',', ' ');
		std::replace(value.begin(), value.end(), ';', ' ');
		std::istringstream entries(value);
		for (std::string entry; entries >> entry;)
		{
			std::uint64_t steamId = 0;
			const auto parsed = std::from_chars(entry.data(), entry.data() + entry.size(), steamId);
			if (parsed.ec == std::errc() && parsed.ptr == entry.data() + entry.size() && steamId != 0)
			{
				steamIds.push_back(steamId);
			}
			else
			{
				++rejectedWhitelistEntries;
			}
		}
		std::sort(steamIds.begin(), steamIds.end());
		const std::size_t parsedEntries = steamIds.size();
		steamIds.erase(std::unique(steamIds.begin(), steamIds.end()), steamIds.end());
		duplicateWhitelistEntries = parsedEntries - steamIds.size();
		BumpRevision();
	}

	struct Configuration
	{
		CConVar<bool> enabled {"cs2ac_enabled", FCVAR_NONE, "Enable or disable CS2AC", true};
		CConVar<bool> aimbotEnabled {"cs2ac_aimbot_enabled", FCVAR_NONE, "Detect damaging visible aim snaps", true};
		CConVar<bool> aimlockEnabled {"cs2ac_aimlock_enabled", FCVAR_NONE, "Detect unnaturally precise target tracking", true};
		CConVar<bool> antiaimEnabled {"cs2ac_antiaim_enabled", FCVAR_NONE, "Detect impossible or manipulated view angles", true};
		CConVar<bool> autostrafeEnabled {"cs2ac_autostrafe_enabled", FCVAR_NONE, "Detect automated air strafing", true};
		CConVar<bool> bhopEnabled {"cs2ac_bhop_enabled", FCVAR_NONE, "Detect automated bunny hopping", true};
		CConVar<bool> dllInjectionEnabled {"cs2ac_dll_injection_enabled", FCVAR_NONE, "Detect suspicious client event subscriptions", true};
		CConVar<bool> desubtickingEnabled {"cs2ac_desubticking_enabled", FCVAR_NONE, "Detect commands that remove normal subtick timing", true};
		CConVar<bool> doubletapEnabled {"cs2ac_doubletap_enabled", FCVAR_NONE, "Detect impossible rapid fire", true};
		CConVar<bool> hyperscrollEnabled {"cs2ac_hyperscroll_enabled", FCVAR_NONE, "Detect automated jump-input frequency", true};
		CConVar<bool> inhumanAccuracyEnabled {"cs2ac_inhuman_accuracy_enabled", FCVAR_NONE, "Detect sustained near-perfect accuracy", true};
		CConVar<bool> invalidCvarEnabled {"cs2ac_invalid_cvar_enabled", FCVAR_NONE, "Detect unsafe client settings", true};
		CConVar<bool> invalidInputEnabled {"cs2ac_invalid_input_enabled", FCVAR_NONE, "Detect malformed player commands", true};
		CConVar<bool> irregularBehaviorEnabled {"cs2ac_irregular_behavior_enabled", FCVAR_NONE,
											   "Detect repeated success with unusually difficult shots", true};
		CConVar<bool> namechangerEnabled {"cs2ac_namechanger_enabled", FCVAR_NONE, "Detect repeated player name changes", true};
		CConVar<bool> nullsEnabled {"cs2ac_nulls_enabled", FCVAR_NONE, "Detect automated opposite-direction input timing", true};
		CConVar<bool> silentaimEnabled {"cs2ac_silentaim_enabled", FCVAR_NONE,
									  "Detect damaging shots that disagree with the visible aim", true};
		CConVar<bool> subtickSpamEnabled {"cs2ac_subtick_spam_enabled", FCVAR_NONE,
										 "Detect repeated same-time button aliases carrying pitch or yaw changes", true};
		CConVar<bool> chatAnnouncements {"cs2ac_chat_announcements", FCVAR_NONE, "Show CS2AC detections in public chat", true};
		CConVar<bool> centerAnnouncements {"cs2ac_center_announcements", FCVAR_NONE,
										 "Show CS2AC detections in the center of the screen", true};
		CConVar<CUtlString> punishmentCommand {"cs2ac_punishment_command", FCVAR_NONE, "Command run for permanent-ban detections",
											 CUtlString("css_addban {steamid64} 0 CS2AC: {detection}")};
		CConVar<CUtlString> kickCommand {"cs2ac_kick_command", FCVAR_NONE, "Command run for kick-only detections",
									   CUtlString("css_kick #{userid} CS2AC: {detection}")};
		CConVar<CUtlString> webhookUrl {"cs2ac_webhook_url", FCVAR_PROTECTED, "Discord webhook URL for detection reports", CUtlString("")};
		CConVar<CUtlString> webhookRoleId {"cs2ac_webhook_role_id", FCVAR_NONE, "Discord role ID mentioned in detection reports",
										 CUtlString("")};
		CConVar<CUtlString> webhookServerAddress {"cs2ac_webhook_server_address", FCVAR_NONE,
												 "Public server address shown in Discord reports", CUtlString("")};
		CConVar<CUtlString> webhookLogoUrl {"cs2ac_webhook_logo_url", FCVAR_NONE,
										  "Public HTTPS URL for the logo shown in Discord reports", CUtlString("")};
		CConVar<CUtlString> whitelist {"cs2ac_whitelist", FCVAR_NONE, "SteamID64s that CS2AC may detect but never punish",
									 CUtlString(""), OnWhitelistChanged};
	};

	Configuration *configuration {};

	bool DetectionSetting(DetectionType detection)
	{
		if (!configuration)
		{
			return false;
		}
		switch (detection)
		{
			case DetectionType::Aimbot:
				return configuration->aimbotEnabled.GetBool();
			case DetectionType::Aimlock:
				return configuration->aimlockEnabled.GetBool();
			case DetectionType::AntiAim:
				return configuration->antiaimEnabled.GetBool();
			case DetectionType::Autostrafe:
				return configuration->autostrafeEnabled.GetBool();
			case DetectionType::Bhop:
				return configuration->bhopEnabled.GetBool();
			case DetectionType::DllInjection:
				return configuration->dllInjectionEnabled.GetBool();
			case DetectionType::Desubticking:
				return configuration->desubtickingEnabled.GetBool();
			case DetectionType::Doubletap:
				return configuration->doubletapEnabled.GetBool();
			case DetectionType::Hyperscroll:
				return configuration->hyperscrollEnabled.GetBool();
			case DetectionType::InhumanAccuracy:
				return configuration->inhumanAccuracyEnabled.GetBool();
			case DetectionType::InvalidCvar:
				return configuration->invalidCvarEnabled.GetBool();
			case DetectionType::InvalidInput:
				return configuration->invalidInputEnabled.GetBool();
			case DetectionType::IrregularBehavior:
				return configuration->irregularBehaviorEnabled.GetBool();
			case DetectionType::NameChanger:
				return configuration->namechangerEnabled.GetBool();
			case DetectionType::Nulls:
				return configuration->nullsEnabled.GetBool();
			case DetectionType::SilentAim:
				return configuration->silentaimEnabled.GetBool();
			case DetectionType::SubtickSpam:
				return configuration->subtickSpamEnabled.GetBool();
			case DetectionType::Count:
				return false;
		}
		return false;
	}
} // namespace

bool settings::Initialize()
{
	if (!configuration)
	{
		configuration = new (std::nothrow) Configuration;
	}
	return configuration != nullptr;
}

void settings::Shutdown()
{
	delete configuration;
	configuration = nullptr;
	WhitelistedSteamIds().clear();
	rejectedWhitelistEntries = 0;
	duplicateWhitelistEntries = 0;
}

bool settings::IsPluginEnabled()
{
	return configuration && configuration->enabled.GetBool();
}

bool settings::IsDetectionEnabled(DetectionType detection)
{
	return IsPluginEnabled() && DetectionSetting(detection);
}

bool settings::IsPlayerWhitelisted(std::uint64_t steamId)
{
	const auto &steamIds = WhitelistedSteamIds();
	return steamId != 0 && std::binary_search(steamIds.begin(), steamIds.end(), steamId);
}

std::size_t settings::GetWhitelistCount()
{
	return WhitelistedSteamIds().size();
}

std::size_t settings::GetRejectedWhitelistCount()
{
	return rejectedWhitelistEntries;
}

std::size_t settings::GetDuplicateWhitelistCount()
{
	return duplicateWhitelistEntries;
}

std::size_t settings::GetEnabledDetectionCount()
{
	const std::uint64_t mask = GetDetectionMask();
	std::size_t count = 0;
	for (std::uint8_t index = 0; index < static_cast<std::uint8_t>(DetectionType::Count); ++index)
	{
		count += (mask >> index) & 1;
	}
	return count;
}

std::uint64_t settings::GetDetectionMask()
{
	if (!IsPluginEnabled())
	{
		return 0;
	}

	std::uint64_t mask = 0;
	for (std::uint8_t index = 0; index < static_cast<std::uint8_t>(DetectionType::Count); ++index)
	{
		const auto detection = static_cast<DetectionType>(index);
		if (DetectionSetting(detection))
		{
			mask |= std::uint64_t {1} << index;
		}
	}
	return mask;
}

std::uint64_t settings::GetRevision()
{
	return settingsRevision;
}

bool settings::ShowChatAnnouncements()
{
	return configuration && configuration->chatAnnouncements.GetBool();
}

bool settings::ShowCenterAnnouncements()
{
	return configuration && configuration->centerAnnouncements.GetBool();
}

const char *settings::GetPunishmentCommand()
{
	return configuration ? configuration->punishmentCommand.Get().Get() : "";
}

const char *settings::GetKickCommand()
{
	return configuration ? configuration->kickCommand.Get().Get() : "";
}

const char *settings::GetWebhookUrl()
{
	return configuration ? configuration->webhookUrl.Get().Get() : "";
}

const char *settings::GetWebhookRoleId()
{
	return configuration ? configuration->webhookRoleId.Get().Get() : "";
}

const char *settings::GetWebhookServerAddress()
{
	return configuration ? configuration->webhookServerAddress.Get().Get() : "";
}

const char *settings::GetWebhookLogoUrl()
{
	return configuration ? configuration->webhookLogoUrl.Get().Get() : "";
}

void settings::MarkConfigReloaded()
{
	BumpRevision();
}

void settings::ExecuteConfig()
{
	if (interfaces::pEngine)
	{
		interfaces::pEngine->ServerCommand("exec cs2ac.cfg\n");
	}
}

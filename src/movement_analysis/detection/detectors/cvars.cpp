// Detects client settings that are unsafe, impossible, or protected by sv_cheats.
#include "common.h"
#include "convar.h"
#include "clientcvar/iclientcvarvalue.h"
#include "movement_analysis/detection/movement_detection.h"
#include "movement_analysis/events/movement_events.h"
#include "settings.h"
#include "utils/ctimer.h"
#include "tier1/random.h"

CConVarRef<bool> sv_cheats("sv_cheats");
CConVar<bool> cs2ac_allow_sv_cheats_testing("cs2ac_allow_sv_cheats_testing", FCVAR_NONE, "Keep CS2AC detections enabled while sv_cheats is enabled",
											false);

// Give replicated cheat-protected settings time to return to normal after sv_cheats is disabled.

bool MovementDetectionService::ShouldRunDetections() const
{
	if (!settings::IsPluginEnabled())
	{
		return false;
	}
	if (!sv_cheats.IsValidRef() || !sv_cheats.IsConVarDataAvailable())
	{
		return true;
	}
	return !sv_cheats.Get() || cs2ac_allow_sv_cheats_testing.Get();
}

bool MovementDetectionService::IsSvCheatsTestingAllowed()
{
	return cs2ac_allow_sv_cheats_testing.Get();
}

bool MovementDetectionService::ShouldCheckClientCvars() const
{
	return ShouldRunDetections() && settings::IsDetectionEnabled(DetectionType::InvalidCvar);
}

extern IClientCvarValue *g_pClientCvarValue;

#define INTEGRITY_CHECK_MIN_INTERVAL    1.0f
#define INTEGRITY_CHECK_MAX_INTERVAL    5.0f
#define MINIMUM_FPS_MAX                 64.0f
#define MAXIMUM_M_YAW                   0.3f
#define SV_CHEATS_MAX_PROPAGATION_DELAY 30.0f

static_global const char *cvarNames[] = {
	"m_yaw",          // Client-controlled, but unsafe outside the accepted range.
	"fps_max",        // Expected to stay at or above the server tick rate.
	"sv_cheats",      // replicated
	"sensitivity",    // capped (0.0001f => 8.0f)
	"cl_showpos",     // cheat (default 0)
	"cam_showangles", // cheat (default 0)
	"cl_drawhud",     // cheat (default 1)
	"cl_pitchdown",   // should always be 89.0f
	"cl_pitchup",     // should always be 89.0f
	"cl_yawspeed",    // should always be 210.0f
	"fov_cs_debug"    // cheat (default 0)
};

static_global const char *userInfoCvarNames[] = {
	"sensitivity",
	"m_yaw",
};

static_global f64 cheatCvarCheckerGraceUntil = -1.0f;

static_function bool ShouldEnforceCheatCvars()
{
	assert(sv_cheats.IsValidRef() && sv_cheats.IsConVarDataAvailable());
	if (sv_cheats.Get())
	{
		return false;
	}
	time_t unixTime = 0;
	time(&unixTime);
	return cheatCvarCheckerGraceUntil < 0 || (f64)unixTime > cheatCvarCheckerGraceUntil;
}

static_function void ResetTransientDetectionStateForAllPlayers()
{
	if (!g_pCS2ACPlayerManager)
	{
		return;
	}
	for (u32 i = 1; i <= MAXPLAYERS; ++i)
	{
		CS2ACPlayer *player = g_pCS2ACPlayerManager->ToPlayer(i);
		if (player && player->movementDetection)
		{
			player->movementDetection->ResetTransientDetectionState();
		}
	}
}

static_global void OnCvarChanged(ConVarRefAbstract *ref, CSplitScreenSlot, const char *pNewValue, const char *, void *)
{
	if (!ref)
	{
		return;
	}
	const bool svCheatsChanged = CS2AC_STREQI(ref->GetName(), "sv_cheats");
	if (!svCheatsChanged && !CS2AC_STREQI(ref->GetName(), "cs2ac_allow_sv_cheats_testing"))
	{
		return;
	}
	if (svCheatsChanged)
	{
		assert(sv_cheats.IsValidRef() && sv_cheats.IsConVarDataAvailable());
		const bool enabled = pNewValue ? CS2AC_STREQI(pNewValue, "true") || (utils::IsNumeric(pNewValue) && atof(pNewValue) != 0.0) : sv_cheats.Get();
		if (!enabled)
		{
			time_t unixTime = 0;
			time(&unixTime);
			cheatCvarCheckerGraceUntil = (f64)unixTime + SV_CHEATS_MAX_PROPAGATION_DELAY;
		}
	}
	ResetTransientDetectionStateForAllPlayers();
}

void MovementDetectionService::InitSvCheatsWatcher()
{
	sv_cheats = CConVarRef<bool>(ConVarRefAbstract("sv_cheats", true));
	g_pCVar->InstallGlobalChangeCallback(OnCvarChanged);
	assert(sv_cheats.IsValidRef() && sv_cheats.IsConVarDataAvailable());
	if (!sv_cheats.Get())
	{
		time_t unixTime = 0;
		time(&unixTime);
		cheatCvarCheckerGraceUntil = (f64)unixTime + SV_CHEATS_MAX_PROPAGATION_DELAY;
	}
}

void MovementDetectionService::CleanupSvCheatsWatcher()
{
	g_pCVar->RemoveGlobalChangeCallback(OnCvarChanged);
}

static_function void ValidateQueriedCvar(CPlayerSlot nSlot, ECvarValueStatus eStatus, const char *pszCvarName, const char *pszCvarValue)
{
	if (eStatus != ECvarValueStatus::ValueIntact)
	{
		return;
	}
	CS2ACPlayer *player = g_pCS2ACPlayerManager->ToPlayer(nSlot);
	if (!player || !player->IsInGame() || !player->movementDetection->ShouldCheckClientCvars())
	{
		return;
	}
	bool invalid = false;
	auto markInvalid = [&](const std::string &reason)
	{
		if (CS2AC_STREQI(pszCvarName, "m_yaw") || CS2AC_STREQI(pszCvarName, "sensitivity"))
		{
			player->movementDetection->MarkCvarSource(pszCvarName, reason, true, false);
		}
		else
		{
			player->movementDetection->MarkInvalidCvar(pszCvarName, reason);
		}
		invalid = true;
	};
	if (CS2AC_STREQI(pszCvarName, "m_yaw"))
	{
		if (!utils::IsNumeric(pszCvarValue) || atof(pszCvarValue) > MAXIMUM_M_YAW)
		{
			markInvalid("m_yaw is invalid or higher than 0.3");
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "fps_max"))
	{
		if (!utils::IsNumeric(pszCvarValue))
		{
			markInvalid("fps_max is invalid");
		}
		else
		{
			f64 fps = atof(pszCvarValue);
			if (player->movementDetection->currentMaxFps == 0.0f)
			{
				player->movementDetection->currentMaxFps = fps;
			}

			if (fps > 0.0f && fps < MINIMUM_FPS_MAX)
			{
				markInvalid("fps_max is lower than 64");
			}
			else if (fabs(fps - player->movementDetection->currentMaxFps) > 0.01f)
			{
				// Joining can briefly change fps_max, so give new players a minute before treating later changes as invalid.
				if (player->movementEvents->GetTimeInServer() < 60.0f)
				{
					player->movementDetection->currentMaxFps = fps;
				}
				else
				{
					std::string reason =
						"Changed fps_max from " + std::to_string(player->movementDetection->currentMaxFps) + " to " + std::to_string(fps);
					markInvalid(reason);
				}
			}
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "sv_cheats"))
	{
		if (!ShouldEnforceCheatCvars())
		{
			return;
		}
		// Allow time for the server's disabled value to reach the client before checking it.
		if (CS2AC_STREQI(pszCvarValue, "1") || CS2AC_STREQI(pszCvarValue, "true"))
		{
			std::string reason = "sv_cheats is enabled on client despite being disabled on server";
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "sensitivity"))
	{
		const bool numeric = utils::IsNumeric(pszCvarValue);
		const f32 sens = numeric ? atof(pszCvarValue) : 0.0f;
		// The actual upper bound is 8.0f but this is to account for possible future updates.
		if (!numeric || sens < 0.0001f || sens > 20.0f)
		{
			std::string reason = "Invalid sensitivity value: " + std::string(pszCvarValue);
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "cl_showpos") || CS2AC_STREQI(pszCvarName, "cam_showangles"))
	{
		if (!ShouldEnforceCheatCvars())
		{
			return;
		}
		if (!CS2AC_STREQI(pszCvarValue, "0") && !CS2AC_STREQI(pszCvarValue, "false"))
		{
			std::string reason = std::string(pszCvarName) + " is enabled on client despite sv_cheats being disabled on server";
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "cl_drawhud"))
	{
		if (!ShouldEnforceCheatCvars())
		{
			return;
		}
		if (CS2AC_STREQI(pszCvarValue, "0") || CS2AC_STREQI(pszCvarValue, "false"))
		{
			std::string reason = "cl_drawhud is disabled on client despite sv_cheats being disabled on server";
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "fov_cs_debug"))
	{
		if (!ShouldEnforceCheatCvars())
		{
			return;
		}
		const f64 fovValue = utils::IsNumeric(pszCvarValue) ? atof(pszCvarValue) : NAN;
		if (!std::isfinite(fovValue) || fovValue != 0.0f)
		{
			std::string reason = "fov_cs_debug is enabled on client despite sv_cheats being disabled on server";
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "cl_pitchdown"))
	{
		if (!utils::IsNumeric(pszCvarValue) || atof(pszCvarValue) != 89.0f)
		{
			std::string reason = "cl_pitchdown has invalid value: " + std::string(pszCvarValue);
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "cl_pitchup"))
	{
		if (!utils::IsNumeric(pszCvarValue) || atof(pszCvarValue) != 89.0f)
		{
			std::string reason = "cl_pitchup has invalid value: " + std::string(pszCvarValue);
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "cl_yawspeed"))
	{
		if (!utils::IsNumeric(pszCvarValue) || atof(pszCvarValue) != 210.0f)
		{
			std::string reason = "cl_yawspeed has invalid value: " + std::string(pszCvarValue);
			markInvalid(reason);
		}
	}
	if (!invalid)
	{
		if (CS2AC_STREQI(pszCvarName, "m_yaw") || CS2AC_STREQI(pszCvarName, "sensitivity"))
		{
			player->movementDetection->MarkCvarSource(pszCvarName, "", false, false);
		}
		else
		{
			player->movementDetection->MarkValidCvar(pszCvarName);
		}
	}
}

static_function f64 CheckUserInfoCvars(CS2ACPlayer *player)
{
	if (!player->movementDetection->ShouldRunDetections())
	{
		return RandomFloat(INTEGRITY_CHECK_MIN_INTERVAL, INTEGRITY_CHECK_MAX_INTERVAL);
	}
	for (auto &name : userInfoCvarNames)
	{
		const char *value = interfaces::pEngine->GetClientConVarValue(player->GetPlayerSlot(), name);
		if (value == nullptr || CS2AC_STREQI(value, ""))
		{
			continue;
		}
		if (CS2AC_STREQI(name, "m_yaw"))
		{
			if (!utils::IsNumeric(value) || atof(value) > MAXIMUM_M_YAW)
			{
				player->movementDetection->MarkCvarSource(name, "m_yaw is invalid", true, true);
			}
			else
			{
				player->movementDetection->MarkCvarSource(name, "", false, true);
			}
		}
		else if (CS2AC_STREQI(name, "sensitivity"))
		{
			const bool numeric = utils::IsNumeric(value);
			const f32 sens = numeric ? atof(value) : 0.0f;
			// The actual upper bound is 8.0f but this is to account for possible future updates.
			if (!numeric || sens < 0.0001f || sens > 20.0f)
			{
				std::string reason = "Invalid sensitivity value: " + std::string(value);
				player->movementDetection->MarkCvarSource(name, reason, true, true);
			}
			else
			{
				player->movementDetection->MarkCvarSource(name, "", false, true);
			}
		}
	}
	return RandomFloat(INTEGRITY_CHECK_MIN_INTERVAL, INTEGRITY_CHECK_MAX_INTERVAL);
}

static_function f64 CheckClientCvars(CPlayerUserId userID)
{
	CS2ACPlayer *player = g_pCS2ACPlayerManager->ToPlayer(userID);
	if (!player || !player->IsInGame())
	{
		return 0.0f;
	}
	if (!player->movementDetection->ShouldRunDetections())
	{
		return RandomFloat(INTEGRITY_CHECK_MIN_INTERVAL, INTEGRITY_CHECK_MAX_INTERVAL);
	}
	CheckUserInfoCvars(player);
	if (!g_pClientCvarValue || !player->movementDetection->ShouldCheckClientCvars())
	{
		return RandomFloat(INTEGRITY_CHECK_MIN_INTERVAL, INTEGRITY_CHECK_MAX_INTERVAL);
	}
	for (auto &name : cvarNames)
	{
		g_pClientCvarValue->QueryCvarValue(player->GetPlayerSlot(), name, ValidateQueriedCvar);
	}
	return RandomFloat(INTEGRITY_CHECK_MIN_INTERVAL, INTEGRITY_CHECK_MAX_INTERVAL);
}

void MovementDetectionService::InitCvarMonitor()
{
	if (cvarMonitorStarted || this->player->IsFakeClient() || this->player->IsCSTV())
	{
		return;
	}
	cvarMonitorStarted = true;
	StartTimer<CPlayerUserId>(CheckClientCvars, this->player->GetClient()->GetUserID(),
							  RandomFloat(INTEGRITY_CHECK_MIN_INTERVAL, INTEGRITY_CHECK_MAX_INTERVAL), true, true);
}

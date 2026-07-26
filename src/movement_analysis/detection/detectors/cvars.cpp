// Detects client settings that are unsafe, impossible, or protected by sv_cheats.
#include "common.h"
#include "convar.h"
#include "clientcvar/iclientcvarvalue.h"
#include "movement_analysis/detection/movement_detection.h"
#include "settings.h"
#include "utils/ctimer.h"
#include "tier1/random.h"
#include <chrono>

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

#define INTEGRITY_CHECK_MIN_INTERVAL 1.0f
#define INTEGRITY_CHECK_MAX_INTERVAL 5.0f
#define MINIMUM_FPS_MAX              64.0f
#define MAXIMUM_M_YAW                0.3f

static constexpr auto SV_CHEATS_MAX_PROPAGATION_DELAY = std::chrono::seconds(30);

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

static_global auto cheatCvarCheckerGraceUntil = (std::chrono::steady_clock::time_point::min)();

static_function bool ShouldEnforceCheatCvars()
{
	assert(sv_cheats.IsValidRef() && sv_cheats.IsConVarDataAvailable());
	if (sv_cheats.Get())
	{
		return false;
	}
	return std::chrono::steady_clock::now() > cheatCvarCheckerGraceUntil;
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
			cheatCvarCheckerGraceUntil = std::chrono::steady_clock::now() + SV_CHEATS_MAX_PROPAGATION_DELAY;
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
		cheatCvarCheckerGraceUntil = std::chrono::steady_clock::now() + SV_CHEATS_MAX_PROPAGATION_DELAY;
	}
}

void MovementDetectionService::CleanupSvCheatsWatcher()
{
	g_pCVar->RemoveGlobalChangeCallback(OnCvarChanged);
	cheatCvarCheckerGraceUntil = (std::chrono::steady_clock::time_point::min)();
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
	auto markInvalid = [&](const std::string &reason, bool kickOnly = false)
	{
		if (CS2AC_STREQI(pszCvarName, "m_yaw") || CS2AC_STREQI(pszCvarName, "sensitivity"))
		{
			player->movementDetection->MarkCvarSource(pszCvarName, reason, true, false, kickOnly);
		}
		else
		{
			player->movementDetection->MarkInvalidCvar(pszCvarName, reason, kickOnly);
		}
		invalid = true;
	};
	if (CS2AC_STREQI(pszCvarName, "m_yaw"))
	{
		if (!utils::IsNumeric(pszCvarValue))
		{
			markInvalid("m_yaw did not return a valid finite number.");
		}
		else if (const f64 yaw = atof(pszCvarValue); yaw > MAXIMUM_M_YAW)
		{
			markInvalid(tinyformat::format("m_yaw is %.6g, but the maximum allowed value is 0.3.", yaw), true);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "fps_max"))
	{
		if (!utils::IsNumeric(pszCvarValue))
		{
			markInvalid("fps_max did not return a valid finite number.");
		}
		else
		{
			const f64 fps = atof(pszCvarValue);
			player->movementDetection->currentMaxFps = fps;
			if (fps > 0.0f && fps < MINIMUM_FPS_MAX)
			{
				markInvalid(tinyformat::format("fps_max is %.6g, but it must be at least 64 or 0 for unlimited.", fps), true);
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
		const bool numeric = utils::IsNumeric(pszCvarValue);
		const bool disabled = CS2AC_STREQI(pszCvarValue, "false") || (numeric && atof(pszCvarValue) == 0.0);
		if (!disabled)
		{
			markInvalid("sv_cheats is enabled or invalid on the client while it is disabled on the server.");
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "sensitivity"))
	{
		const bool numeric = utils::IsNumeric(pszCvarValue);
		const f32 sens = numeric ? atof(pszCvarValue) : 0.0f;
		// The actual upper bound is 8.0f but this is to account for possible future updates.
		if (!numeric || sens < 0.0001f || sens > 20.0f)
		{
			std::string reason = numeric ? tinyformat::format("sensitivity is %.6g, but it must be between 0.0001 and 20.", sens)
										 : "sensitivity did not return a valid finite number.";
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
			std::string reason = std::isfinite(fovValue) ? tinyformat::format("fov_cs_debug is %.6g, but it must be 0.", fovValue)
														 : "fov_cs_debug did not return a valid finite number.";
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "cl_pitchdown"))
	{
		const bool numeric = utils::IsNumeric(pszCvarValue);
		const f64 value = numeric ? atof(pszCvarValue) : 0.0;
		if (!numeric || value != 89.0)
		{
			std::string reason = numeric ? tinyformat::format("cl_pitchdown is %.6g, but it must be 89.", value)
										 : "cl_pitchdown did not return a valid finite number.";
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "cl_pitchup"))
	{
		const bool numeric = utils::IsNumeric(pszCvarValue);
		const f64 value = numeric ? atof(pszCvarValue) : 0.0;
		if (!numeric || value != 89.0)
		{
			std::string reason =
				numeric ? tinyformat::format("cl_pitchup is %.6g, but it must be 89.", value) : "cl_pitchup did not return a valid finite number.";
			markInvalid(reason);
		}
	}
	else if (CS2AC_STREQI(pszCvarName, "cl_yawspeed"))
	{
		const bool numeric = utils::IsNumeric(pszCvarValue);
		const f64 value = numeric ? atof(pszCvarValue) : 0.0;
		if (!numeric || value != 210.0)
		{
			std::string reason =
				numeric ? tinyformat::format("cl_yawspeed is %.6g, but it must be 210.", value) : "cl_yawspeed did not return a valid finite number.";
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

static_function void CheckUserInfoCvars(CS2ACPlayer *player)
{
	for (auto &name : userInfoCvarNames)
	{
		const char *value = interfaces::pEngine->GetClientConVarValue(player->GetPlayerSlot(), name);
		if (value == nullptr || CS2AC_STREQI(value, ""))
		{
			continue;
		}
		if (CS2AC_STREQI(name, "m_yaw"))
		{
			if (!utils::IsNumeric(value))
			{
				player->movementDetection->MarkCvarSource(name, "m_yaw did not return a valid finite number.", true, true);
			}
			else if (const f64 yaw = atof(value); yaw > MAXIMUM_M_YAW)
			{
				player->movementDetection->MarkCvarSource(name, tinyformat::format("m_yaw is %.6g, but the maximum allowed value is 0.3.", yaw), true,
														  true, true);
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
				std::string reason = numeric ? tinyformat::format("sensitivity is %.6g, but it must be between 0.0001 and 20.", sens)
											 : "sensitivity did not return a valid finite number.";
				player->movementDetection->MarkCvarSource(name, reason, true, true);
			}
			else
			{
				player->movementDetection->MarkCvarSource(name, "", false, true);
			}
		}
	}
}

static_function f64 CheckClientCvars(CPlayerUserId userID)
{
	CS2ACPlayer *player = g_pCS2ACPlayerManager->ToPlayer(userID);
	if (!player || !player->IsInGame())
	{
		return 0.0f;
	}
	if (!player->movementDetection->ShouldCheckClientCvars())
	{
		return RandomFloat(INTEGRITY_CHECK_MIN_INTERVAL, INTEGRITY_CHECK_MAX_INTERVAL);
	}
	CheckUserInfoCvars(player);
	if (!g_pClientCvarValue)
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

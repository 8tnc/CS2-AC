#include "player_context.h"

#include "cs2ac.h"
#include "movement_analysis/detection/movement_detection.h"
#include "movement_analysis/jump_analysis/jump_analysis.h"
#include "movement_analysis/settings/movement_settings.h"
#include "movement_analysis/events/movement_events.h"

CS2ACPlayer::~CS2ACPlayer()
{
	delete movementDetection;
	delete jumpAnalysis;
	delete movementEvents;
}

void CS2ACPlayer::Init()
{
	delete movementDetection;
	delete jumpAnalysis;
	delete movementEvents;
	movementDetection = new MovementDetectionService(this);
	jumpAnalysis = new JumpAnalysisService(this);
	movementEvents = new MovementEventService(this);
	Reset();
}

void CS2ACPlayer::Reset()
{
	MovementPlayer::Reset();
	lastTeleportTime = 0.0;
	if (movementDetection)
	{
		movementDetection->Reset();
	}
	if (jumpAnalysis)
	{
		jumpAnalysis->Reset();
	}
	if (movementEvents)
	{
		movementEvents->Reset();
	}
}

void CS2ACPlayer::OnPlayerFullyConnect()
{
	movementDetection->OnPlayerFullyConnect();
}

void CS2ACPlayer::OnPhysicsSimulate()
{
	MovementPlayer::OnPhysicsSimulate();
}

void CS2ACPlayer::OnPhysicsSimulatePost()
{
	MovementPlayer::OnPhysicsSimulatePost();
	movementDetection->OnPhysicsSimulatePost();
}

void CS2ACPlayer::OnProcessUsercmds(PlayerCommand *commands, int numCommands)
{
	g_CS2AC.OnProcessUsercmds(this, commands, numCommands);
}

void CS2ACPlayer::OnSetupMove(PlayerCommand *command)
{
	g_CS2AC.OnSetupMove(this, command);
	movementDetection->OnSetupMove(command);
}

void CS2ACPlayer::OnProcessMovement()
{
	MovementPlayer::OnProcessMovement();
	movementDetection->OnProcessMovement();
	jumpAnalysis->OnProcessMovement();
}

void CS2ACPlayer::OnProcessMovementPost()
{
	movementDetection->OnProcessMovementPost();
	jumpAnalysis->UpdateJump();
	jumpAnalysis->OnProcessMovementPost();
	MovementPlayer::OnProcessMovementPost();
}

void CS2ACPlayer::OnDuck() {}

void CS2ACPlayer::OnJumpLegacy()
{
	movementEvents->OnJumpLegacy();
}

void CS2ACPlayer::OnJumpLegacyPost()
{
	movementEvents->OnJumpLegacyPost();
}

void CS2ACPlayer::OnJumpModern()
{
	movementEvents->OnJumpModern();
}

void CS2ACPlayer::OnJumpModernPost()
{
	movementEvents->OnJumpModernPost();
}

void CS2ACPlayer::OnAirMove()
{
	movementDetection->OnAirMove();
}

void CS2ACPlayer::OnAirAccelerate(Vector &, f32 &, f32 &)
{
	jumpAnalysis->OnAirAccelerate();
}

void CS2ACPlayer::OnAirAcceleratePost(Vector wishdir, f32 wishspeed, f32 accel)
{
	jumpAnalysis->OnAirAcceleratePost(wishdir, wishspeed, accel);
}

void CS2ACPlayer::OnTryPlayerMove(Vector *, trace_t *, bool *)
{
	jumpAnalysis->OnTryPlayerMove();
}

void CS2ACPlayer::OnTryPlayerMovePost(Vector *, trace_t *, bool *)
{
	jumpAnalysis->OnTryPlayerMovePost();
}

void CS2ACPlayer::OnStartTouchGround()
{
	movementDetection->CreateLandEvent();
	jumpAnalysis->EndJump();
}

void CS2ACPlayer::OnStopTouchGround()
{
	if (!inPerf && !GetMovementSetting(MOVEMENT_SETTING_SV_LEGACY_JUMP)->m_bValue)
	{
		f32 landingTick = GetMoveServices()->m_ModernJump().m_nLastLandedTick() + GetMoveServices()->m_ModernJump().m_flLastLandedFrac();
		f32 window = GetMovementSetting(MOVEMENT_SETTING_SV_BHOP_TIME_WINDOW)->m_fl32Value * 0.5f * ENGINE_FIXED_TICK_RATE;
		f32 startTime = currentMoveData->m_flSubtickStartFraction + currentMoveData->m_nTickCount;
		inPerf = startTime >= landingTick - window && startTime <= landingTick + window && jumped;
	}
	jumpAnalysis->AddJump();
}

void CS2ACPlayer::OnChangeMoveType(MoveType_t oldMoveType)
{
	movementDetection->OnChangeMoveType(oldMoveType);
	jumpAnalysis->OnChangeMoveType(oldMoveType);
}

void CS2ACPlayer::OnTeleport(const Vector *, const QAngle *, const Vector *)
{
	lastTeleportTime = g_pCS2ACUtils->GetServerGlobals()->curtime;
	jumpAnalysis->HandleTeleport();
}

bool CS2ACPlayer::JustTeleported(f32 threshold)
{
	return g_pCS2ACUtils->GetServerGlobals()->curtime - lastTeleportTime < threshold;
}

const CVValue_t *CS2ACPlayer::GetMovementSetting(MovementSetting setting)
{
	return movement_settings::settingRefs[setting]->GetConVarData()->ValueOrDefault(-1);
}

void CS2ACPlayer::PrintDebug(const char *format, ...)
{
	char message[1024] {};
	va_list arguments;
	va_start(arguments, format);
	V_vsnprintf(message, sizeof(message), format, arguments);
	va_end(arguments);
	Msg("[CS2AC Nulls] %s\n", message);
}

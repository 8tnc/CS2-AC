#include "movement_analysis/detection/movement_detection.h"
#include "sdk/usercmd.h"
#include <cmath>

#define MAX_ANGLE_FRAME_HISTORY 5

float MovementDetectionService::CalculateYawSpeed(size_t index)
{
	if (index == 0)
	{
		return 0.0f;
	}

	float currentYaw = angleFrameHistory[index].angle.y;
	float lastYaw = angleFrameHistory[index - 1].angle.y;
	float frameTime = angleFrameHistory[index].frametime;

	float yawDiff = currentYaw - lastYaw;
	if (yawDiff > 180.0f)
	{
		yawDiff -= 360.0f;
	}
	if (yawDiff < -180.0f)
	{
		yawDiff += 360.0f;
	}

	return frameTime > 0.0f ? yawDiff / frameTime : 0.0f;
}

float MovementDetectionService::CalculateYawAccel(size_t index)
{
	if (index < 2)
	{
		return 0.0f;
	}

	float currentSpeed = CalculateYawSpeed(index);
	float lastSpeed = CalculateYawSpeed(index - 1);
	float frameTime = angleFrameHistory[index].frametime;

	return (currentSpeed - lastSpeed) / frameTime;
}

void MovementDetectionService::DetectOptimization(PlayerCommand *pc)
{
	const CBaseUserCmdPB &baseCmd = pc->base();
	const float frameTime = g_pCS2ACUtils->GetGlobals()->frametime;
	if (!std::isfinite(frameTime) || frameTime <= 0.0f)
	{
		angleFrameHistory.clear();
		yawAccelPercent = 0.0f;
		return;
	}
	if (!angleFrameHistory.empty() && pc->cmdNum != angleFrameHistory.back().cmdNum + 1)
	{
		angleFrameHistory.clear();
		yawAccelPercent = 0.0f;
	}

	QAngle angles;
	angles.x = baseCmd.viewangles().x();
	angles.y = baseCmd.viewangles().y();
	angles.z = baseCmd.viewangles().z();
	if (!std::isfinite(angles.x) || !std::isfinite(angles.y) || !std::isfinite(angles.z))
	{
		angleFrameHistory.clear();
		yawAccelPercent = 0.0f;
		return;
	}

	angleFrameHistory.push_back({pc->cmdNum, angles, frameTime});
	while (angleFrameHistory.size() > MAX_ANGLE_FRAME_HISTORY)
	{
		angleFrameHistory.pop_front();
	}

	size_t historySize = angleFrameHistory.size();

	bool evaluated = false;
	bool matched = false;
	// check if there are enough angle frames to calculate yaw speed
	if (historySize > 2)
	{
		float lastYawSpeed = CalculateYawSpeed(historySize - 2);
		float currentYawSpeed = CalculateYawSpeed(historySize - 1);
		bool switchedStrafeDirection = std::signbit(currentYawSpeed) != std::signbit(lastYawSpeed);

		// check if there are enough angle frames to calculate yaw accel
		if (historySize >= MAX_ANGLE_FRAME_HISTORY && switchedStrafeDirection)
		{
			float yawAccel2TicksAgo = std::abs(CalculateYawAccel(historySize - 3));
			float lastYawAccel = std::abs(CalculateYawAccel(historySize - 2));
			float currentYawAccel = std::abs(CalculateYawAccel(historySize - 1));
			float lastNextDiff = std::abs(currentYawAccel - yawAccel2TicksAgo);

			if (lastNextDiff < 1.0f)
			{
				evaluated = true;
				float avgAccel = (currentYawAccel + yawAccel2TicksAgo) * 0.5f;
				if (avgAccel < 2.0f && (lastYawAccel - avgAccel) > 2.0f)
				{
					matched = true;
				}
			}
		}
	}
	if (evaluated)
	{
		yawAccelPercent = yawAccelPercent * 0.95f + (matched ? 0.05f : 0.0f);
	}

	// finally check for suspicious yaw accel patterns
	if (yawAccelPercent > 0.9f)
	{
		this->MarkInfraction(MovementDetectionService::Infraction::Type::StrafeHack, "Strafe optimizer detected");
		this->yawAccelPercent = 0.0f;
		this->angleFrameHistory.clear();
		return;
	}
}

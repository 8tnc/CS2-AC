#pragma once

#include "common.h"

namespace hooks
{
	bool Initialize(std::vector<std::string> &missing);
	void Cleanup();
	void HookActivePlayers();
	void ResetMap();
} // namespace hooks

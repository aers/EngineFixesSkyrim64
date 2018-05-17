#pragma once

namespace config
{
	// Form lookup cache + distant tree LOD alpha update form cache
	// equivalent to SSE fixes
	extern bool patchFormCaching;

	// Replace BSReadWriteLock implementation
	extern bool patchBSReadWriteLock;

	// Replace Memory Manager
	extern bool patchMemoryManager;

	// Ports from LE bug fixes mod
	extern bool patchDoublePerkApply;
	extern bool patchSlowTimeCameraMovement;
	extern bool patchVerticalLookSensitivity;

	// decouple waterflow timer from timescale
	extern bool patchWaterflowTimer;
	extern float waterflowTimescale;
	// fix blocky reflections on tree LODs
	extern bool patchTreeReflections;
	// snow sparkle 
	extern bool patchSnowSparkle;

	bool LoadConfig(const std::string& path);
}
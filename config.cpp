#include "../../common/IDebugLog.h"

#include "lib/INIReader.h"

#include "config.h"

namespace config
{
	// Form lookup cache + distant tree LOD alpha update form cache
	// equivalent to SSE fixes
	bool patchFormCaching = true;

	// Replace BSReadWriteLock implementation
	bool patchBSReadWriteLock = false;

	// replace MemoryManager
	bool patchMemoryManager = false;

	// Ports from LE bug fixes mod
	bool patchDoublePerkApply = true;
	bool patchSlowTimeCameraMovement = true;
	bool patchVerticalLookSensitivity = true;

	// decouple waterflow timer from timescale
	bool patchWaterflowTimer = true;
	float waterflowTimescale = 20.0;

	// fix blocky reflections on tree LODs
	bool patchTreeReflections = false;

	// snow sparkle
	bool patchSnowSparkle = true;

	// save sound categories that aren't in the base game to SKSE cosave
	bool patchSaveAddedSoundCategories = true;

	bool LoadConfig(const std::string& path)
	{
		INIReader reader(path);

		if (reader.ParseError() < 0)
		{
			_MESSAGE("unable to read ini file at path %s", path.c_str());
			return false;
		}

		patchFormCaching = reader.GetBoolean("FormCaching", "enabled", true);
		patchBSReadWriteLock = reader.GetBoolean("BSReadWriteLock", "enabled", false);
		patchMemoryManager = reader.GetBoolean("MemoryManager", "enabled", false);
		patchDoublePerkApply = reader.GetBoolean("DoublePerkApply", "enabled", true);
		patchSlowTimeCameraMovement = reader.GetBoolean("SlowTimeCamera", "enabled", true);
		patchVerticalLookSensitivity = reader.GetBoolean("VerticalSensitivity", "enabled", true);
		patchWaterflowTimer = reader.GetBoolean("Waterflow", "enabled", true);
		waterflowTimescale = reader.GetReal("Waterflow", "timescale", 20.0);
		patchTreeReflections = reader.GetBoolean("TreeReflection", "enabled", false);
		patchSnowSparkle = reader.GetBoolean("SnowSparkle", "enabled", true);
		patchSaveAddedSoundCategories = reader.GetBoolean("SaveAddedSoundCategories", "enabled", true);

		return true;
	}
}
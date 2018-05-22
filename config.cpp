#include "../../common/IDebugLog.h"

#include "lib/Simpleini.h"

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
		CSimpleIniA ini;
		SI_Error rc = ini.LoadFile(path.c_str());

		if (rc < 0)
		{
			_MESSAGE("unable to read ini file at path %s", path.c_str());
			return false;
		}

		patchFormCaching = ini.GetBoolValue("FormCaching", "enabled", true);
		patchBSReadWriteLock = ini.GetBoolValue("BSReadWriteLock", "enabled", false);
		patchMemoryManager = ini.GetBoolValue("MemoryManager", "enabled", false);
		patchDoublePerkApply = ini.GetBoolValue("DoublePerkApply", "enabled", true);
		patchSlowTimeCameraMovement = ini.GetBoolValue("SlowTimeCamera", "enabled", true);
		patchVerticalLookSensitivity = ini.GetBoolValue("VerticalSensitivity", "enabled", true);
		patchWaterflowTimer = ini.GetBoolValue("Waterflow", "enabled", true);
		waterflowTimescale = static_cast<float>(ini.GetDoubleValue("Waterflow", "timescale", 20.0));
		patchTreeReflections = ini.GetBoolValue("TreeReflection", "enabled", false);
		patchSnowSparkle = ini.GetBoolValue("SnowSparkle", "enabled", true);
		patchSaveAddedSoundCategories = ini.GetBoolValue("SaveAddedSoundCategories", "enabled", true);

		return true;
	}
}	
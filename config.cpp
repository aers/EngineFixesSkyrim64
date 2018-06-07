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

	// patch maxstdio to 2048 (default 512)
	// see https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setmaxstdio
	// this fixes the false save corruption bug 
	bool patchMaxStdio = true;
	bool stdioDebugMode = false;

	// various save game related patches
	bool patchSaves = true;
	bool fixTaaSaveBugs = true;
	bool regularQuicksaves = false;

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

	// disable chargen precache
	// disabled by default since most people should't need it
	bool patchPrecacheKiller = false;
	
	// terrible code that likely needs to be rewritten
	// deletes leftover SKSE64 cosaves that are not removed when the matching save is removed
	// disabled by default because it's likely really buggy - enable if you want to live on the edge
	bool killOrphanedCosave = false

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
		patchMaxStdio = ini.GetBoolValue("MaxStdio", "enabled", true);
		stdioDebugMode = ini.GetBoolValue("MaxStdio", "debugmode", false);
		patchSaves = ini.GetBoolValue("Saves", "enbabled", true);
		fixTaaSaveBugs = ini.GetBoolValue("Saves", "fixtaasavebugs", true);
		regularQuicksaves = ini.GetBoolValue("Saves", "regularquicksaves", false);
		patchDoublePerkApply = ini.GetBoolValue("DoublePerkApply", "enabled", true);
		patchSlowTimeCameraMovement = ini.GetBoolValue("SlowTimeCamera", "enabled", true);
		patchVerticalLookSensitivity = ini.GetBoolValue("VerticalSensitivity", "enabled", true);
		patchWaterflowTimer = ini.GetBoolValue("Waterflow", "enabled", true);
		waterflowTimescale = static_cast<float>(ini.GetDoubleValue("Waterflow", "timescale", 20.0));
		patchTreeReflections = ini.GetBoolValue("TreeReflection", "enabled", false);
		patchSnowSparkle = ini.GetBoolValue("SnowSparkle", "enabled", true);
		patchSaveAddedSoundCategories = ini.GetBoolValue("SaveAddedSoundCategories", "enabled", true);
		patchPrecacheKiller = ini.GetBoolValue("PrecacheKiller", "enabled", false);
		killOrphanedCosave = ini.GetBoolValue("killOrphanedCosave", "enabled", false);
		return true;
	}
}	
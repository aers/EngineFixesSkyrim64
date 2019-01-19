#include "Simpleini.h"

#include "config.h"

namespace config
{
    bool verboseLogging = false;
    bool cleanSKSECosaves = false;

    // Patches
    bool patchDisableChargenPrecache = false;
    bool patchEnableAchievementsWithMods = true;
    bool patchFormCaching = true;
    bool patchMaxStdio = true;
    bool patchRegularQuicksaves = false;
    bool patchSaveAddedSoundCategories = true;
    bool patchWaterflowAnimation = true;
    bool patchTreeLODReferenceCaching = true;
    float waterflowSpeed = 20.0;

    // Fixes
    bool fixDoublePerkApply = true;
    bool fixMemoryAccessErrors = true;
    bool fixMO5STypo = true;
    bool fixPerkFragmentIsRunning = true;
    bool fixRemovedSpellBook = true;
    bool fixSaveScreenshots = true;
    bool fixSlowTimeCameraMovement = true;
    bool fixTreeReflections = true;

    // Warnings
    bool warnDupeAddonNodes = true;
    bool warnRefHandleLimit = true;
    uint32_t warnRefrMainMenuLimit = 800000;
    uint32_t warnRefrLoadedGameLimit = 1000000;

    // Experimental
    bool experimentalMemoryManager = false;
    bool experimentalUseTBBMalloc = true;
    bool experimentalTreatAllModsAsMasters = false;

    bool LoadConfig(const std::string& path)
    {
        CSimpleIniA ini;
        SI_Error rc = ini.LoadFile(path.c_str());

        if (rc < 0)
        {
            _MESSAGE("unable to read ini file at path %s", path.c_str());
            return false;
        }

        verboseLogging = ini.GetBoolValue("EngineFixes", "VerboseLogging", false);
        cleanSKSECosaves = ini.GetBoolValue("EngineFixes", "CleanSKSECosaves", true);

        // Patches
        patchDisableChargenPrecache = ini.GetBoolValue("Patches", "DisableChargenPrecache", false);
        patchEnableAchievementsWithMods = ini.GetBoolValue("Patches", "EnableAchievementsWithMods", true);
        patchFormCaching = ini.GetBoolValue("Patches", "FormCaching", true);
        patchMaxStdio = ini.GetBoolValue("Patches", "MaxStdio", true);
        patchRegularQuicksaves = ini.GetBoolValue("Patches", "RegularQuicksaves", false);
        patchSaveAddedSoundCategories = ini.GetBoolValue("Patches", "SaveAddedSoundCategories", true);
        patchTreeLODReferenceCaching = ini.GetBoolValue("Patches", "TreeLODReferenceCaching", true);
        patchWaterflowAnimation = ini.GetBoolValue("Patches", "WaterflowAnimation", true);
        waterflowSpeed = static_cast<float>(ini.GetDoubleValue("Patches", "WaterflowSpeed", 20.0));

        // Fixes
        fixDoublePerkApply = ini.GetBoolValue("Fixes", "DoublePerkApply", true);
        fixMemoryAccessErrors = ini.GetBoolValue("Fixes", "MemoryAccessErrors", true);
        fixMO5STypo = ini.GetBoolValue("Fixes", "MO5STypo", true);
        fixPerkFragmentIsRunning = ini.GetBoolValue("Fixes", "PerkFragmentIsRunning", true);
        fixRemovedSpellBook = ini.GetBoolValue("Fixes", "RemovedSpellBook", true);
        fixSaveScreenshots = ini.GetBoolValue("Fixes", "SaveScreenshots", true);
        fixSlowTimeCameraMovement = ini.GetBoolValue("Fixes", "SlowTimeCameraMovement", true);
        fixTreeReflections = ini.GetBoolValue("Fixes", "TreeReflections", true);

        // Warnings
        warnDupeAddonNodes = ini.GetBoolValue("Warnings", "DupeAddonNodes", true);
        warnRefHandleLimit = ini.GetBoolValue("Warnings", "RefHandleLimit", true);
        warnRefrMainMenuLimit = ini.GetLongValue("Warnings", "RefrMainMenuLimit", 800000);
        warnRefrLoadedGameLimit = ini.GetLongValue("Warnings", "RefrLoadedGameLimit", 1000000);

        // Experimental
        experimentalMemoryManager = ini.GetBoolValue("Experimental", "MemoryManager", false);
        experimentalUseTBBMalloc = ini.GetBoolValue("Experimental", "UseTBBMalloc", true);
        experimentalTreatAllModsAsMasters = ini.GetBoolValue("Experimental", "TreatAllModsAsMasters", false);

        return true;
    }
}
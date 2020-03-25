#include "INIReader.h"

#include "config.h"

namespace config
{
    bool verboseLogging = false;
    bool cleanSKSECosaves = true;

    // Patches
    bool patchDisableChargenPrecache = false;
    bool patchEnableAchievementsWithMods = true;
    bool patchFormCaching = true;
    bool patchMaxStdio = true;
    bool patchRegularQuicksaves = false;
    bool patchSaveAddedSoundCategories = true;
    bool patchScrollingDoesntSwitchPOV = false;
    bool patchSleepWaitTime = false;
    float sleepWaitTimeModifier = 0.3F;
    bool patchWaterflowAnimation = true;
    bool patchTreeLODReferenceCaching = true;
    float waterflowSpeed = 20.0;

    // Fixes
    bool fixAnimationLoadSignedCrash = true;
    bool fixArcheryDownwardAiming = true;
    bool fixBethesdaNetCrash = true;
    bool fixBSLightingAmbientSpecular = true;
    bool fixBSLightingShaderForceAlphaTest = true;
    bool fixBSLightingShaderGeometryParallaxBug = true;
    bool fixBSTempEffectNiRTTI = true;
    bool fixCalendarSkipping = true;
    bool fixCellInit = true;
    bool fixConjurationEnchantAbsorbs = true;
    bool fixDoublePerkApply = true;
    bool fixEquipShoutEventSpam = true;
    bool fixGetKeywordItemCount = false;
    bool fixGHeapLeakDetectionCrash = true;
    bool fixLipSync = true;
    bool fixMemoryAccessErrors = true;
    bool fixMO5STypo = true;
    bool fixPerkFragmentIsRunning = true;
    bool fixRemovedSpellBook = true;
    bool fixSaveScreenshots = true;
    bool fixSlowTimeCameraMovement = true;
    bool fixTreeReflections = true;
    bool fixUnequipAllCrash = true;
    bool fixVerticalLookSensitivity = true;
    bool fixWeaponBlockScaling = true;

    // Warnings
    bool warnDupeAddonNodes = true;
    bool warnRefHandleLimit = true;
    uint32_t warnRefrMainMenuLimit = 800000;
    uint32_t warnRefrLoadedGameLimit = 1000000;

    // Experimental
    bool experimentalSaveGameMaxSize = false;
    bool experimentalTreatAllModsAsMasters = false;

    bool LoadConfig(const std::string& path)
    {
        INIReader ini(path);

        if (ini.ParseError() < 0)
        {
            _MESSAGE("unable to read ini file at path %s", path.c_str());
            return false;
        }

        verboseLogging = ini.GetBoolean("EngineFixes", "VerboseLogging", false);
        cleanSKSECosaves = ini.GetBoolean("EngineFixes", "CleanSKSECosaves", true);

        // Patches
        patchDisableChargenPrecache = ini.GetBoolean("Patches", "DisableChargenPrecache", false);
        patchEnableAchievementsWithMods = ini.GetBoolean("Patches", "EnableAchievementsWithMods", true);
        patchFormCaching = ini.GetBoolean("Patches", "FormCaching", true);
        patchMaxStdio = ini.GetBoolean("Patches", "MaxStdio", true);
        patchRegularQuicksaves = ini.GetBoolean("Patches", "RegularQuicksaves", false);
        patchSaveAddedSoundCategories = ini.GetBoolean("Patches", "SaveAddedSoundCategories", true);
        patchScrollingDoesntSwitchPOV = ini.GetBoolean("Patches", "ScrollingDoesntSwitchPOV", false);
        patchSleepWaitTime = ini.GetBoolean("Patches", "SleepWaitTime", false);
        sleepWaitTimeModifier = static_cast<float>(ini.GetReal("Patches", "SleepWaitTimeModifier", 0.3));
        patchTreeLODReferenceCaching = ini.GetBoolean("Patches", "TreeLODReferenceCaching", true);
        patchWaterflowAnimation = ini.GetBoolean("Patches", "WaterflowAnimation", true);
        waterflowSpeed = static_cast<float>(ini.GetReal("Patches", "WaterflowSpeed", 20.0));

        // Fixes
        fixAnimationLoadSignedCrash = ini.GetBoolean("Fixes", "AnimationLoadSignedCrash", true);
        fixArcheryDownwardAiming = ini.GetBoolean("Fixes", "ArcheryDownwardAiming", true);
        fixBethesdaNetCrash = ini.GetBoolean("Fixes", "BethesdaNetCrash", true);
        fixBSLightingAmbientSpecular = ini.GetBoolean("Fixes", "BSLightingAmbientSpecular", true);
        fixBSLightingShaderForceAlphaTest = ini.GetBoolean("Fixes", "BSLightingShaderForceAlphaTest", true);
        fixBSLightingShaderGeometryParallaxBug = ini.GetBoolean("Fixes", "BSLightingShaderParallaxBug", true);
        fixBSTempEffectNiRTTI = ini.GetBoolean("Fixes", "BSTempEffectNiRTTI", true);
        fixCalendarSkipping = ini.GetBoolean("Fixes", "CalendarSkipping", true);
        fixCellInit = ini.GetBoolean("Fixes", "CellInit", true);
        fixConjurationEnchantAbsorbs = ini.GetBoolean("Fixes", "ConjurationEnchantAbsorbs", true);
        fixDoublePerkApply = ini.GetBoolean("Fixes", "DoublePerkApply", true);
        fixEquipShoutEventSpam = ini.GetBoolean("Fixes", "EquipShoutEventSpam", true);
        fixGetKeywordItemCount = ini.GetBoolean("Fixes", "GetKeywordItemCount", false);
        fixGHeapLeakDetectionCrash = ini.GetBoolean("Fixes", "GHeapLeakDetectionCrash", true);
        fixLipSync = ini.GetBoolean("Fixes", "LipSync", true);
        fixMemoryAccessErrors = ini.GetBoolean("Fixes", "MemoryAccessErrors", true);
        fixMO5STypo = ini.GetBoolean("Fixes", "MO5STypo", true);
        fixPerkFragmentIsRunning = ini.GetBoolean("Fixes", "PerkFragmentIsRunning", true);
        fixRemovedSpellBook = ini.GetBoolean("Fixes", "RemovedSpellBook", true);
        fixSaveScreenshots = ini.GetBoolean("Fixes", "SaveScreenshots", true);
        fixSlowTimeCameraMovement = ini.GetBoolean("Fixes", "SlowTimeCameraMovement", true);
        fixTreeReflections = ini.GetBoolean("Fixes", "TreeReflections", true);
        fixUnequipAllCrash = ini.GetBoolean("Fixes", "UnequipAllCrash", true);
        fixVerticalLookSensitivity = ini.GetBoolean("Fixes", "VerticalLookSensitivity", true);
        fixWeaponBlockScaling = ini.GetBoolean("Fixes", "WeaponBlockScaling", true);

        // Warnings
        warnDupeAddonNodes = ini.GetBoolean("Warnings", "DupeAddonNodes", true);
        warnRefHandleLimit = ini.GetBoolean("Warnings", "RefHandleLimit", true);
        warnRefrMainMenuLimit = ini.GetInteger("Warnings", "RefrMainMenuLimit", 800000);
        warnRefrLoadedGameLimit = ini.GetInteger("Warnings", "RefrLoadedGameLimit", 1000000);

        // Experimental
        experimentalSaveGameMaxSize = ini.GetBoolean("Experimental", "SaveGameMaxSize", false);
        experimentalTreatAllModsAsMasters = ini.GetBoolean("Experimental", "TreatAllModsAsMasters", false);

        warnDupeAddonNodes = false;

        return true;
    }
}

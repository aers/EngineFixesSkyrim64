#pragma once

namespace config
{
    extern bool verboseLogging;
    extern bool cleanSKSECosaves;

    // Patches
    extern bool patchDisableChargenPrecache;
    extern bool patchEnableAchievementsWithMods;
    extern bool patchFormCaching;
    extern bool patchMaxStdio;
    extern bool patchRegularQuicksaves;
    extern bool patchSaveAddedSoundCategories;
    extern bool patchScrollingDoesntSwitchPOV;
    extern bool patchSleepWaitTime;
    extern float sleepWaitTimeModifier;
    extern bool patchTreeLODReferenceCaching;
    extern bool patchWaterflowAnimation;
    extern float waterflowSpeed;

    // Fixes
    extern bool fixBethesdaNetCrash;
    extern bool fixDoublePerkApply;
    extern bool fixEquipShoutEventSpam;
    extern bool fixMemoryAccessErrors;
    extern bool fixMO5STypo;
    extern bool fixPerkFragmentIsRunning;
    extern bool fixRemovedSpellBook;
    extern bool fixSaveScreenshots;
    extern bool fixSlowTimeCameraMovement;
    extern bool fixTreeReflections;

    // Warnings
    extern bool warnDupeAddonNodes;
    extern bool warnRefHandleLimit;
    extern uint32_t warnRefrMainMenuLimit;
    extern uint32_t warnRefrLoadedGameLimit;

    // Experimental
    extern bool experimentalMemoryManager;
    extern bool experimentalUseTBBMalloc;
    extern bool experimentalTreatAllModsAsMasters;

    bool LoadConfig(const std::string& path);
}
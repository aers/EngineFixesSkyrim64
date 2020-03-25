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
    extern bool fixArcheryDownwardAiming;
    extern bool fixBethesdaNetCrash;
    extern bool fixBSLightingAmbientSpecular;
    extern bool fixBSLightingShaderForceAlphaTest;
    extern bool fixBSLightingShaderGeometryParallaxBug;
    extern bool fixBSTempEffectNiRTTI;
    extern bool fixCalendarSkipping;
    extern bool fixCellInit;
    extern bool fixConjurationEnchantAbsorbs;
    extern bool fixDoublePerkApply;
    extern bool fixEquipShoutEventSpam;
    extern bool fixGetKeywordItemCount;
    extern bool fixGHeapLeakDetectionCrash;
    extern bool fixLipSync;
    extern bool fixMemoryAccessErrors;
    extern bool fixMO5STypo;
    extern bool fixPerkFragmentIsRunning;
    extern bool fixRemovedSpellBook;
    extern bool fixSaveScreenshots;
    extern bool fixSlowTimeCameraMovement;
    extern bool fixTreeReflections;
    extern bool fixUnequipAllCrash;
    extern bool fixVerticalLookSensitivity;
    extern bool fixAnimationLoadSignedCrash;
    extern bool fixWeaponBlockScaling;

    // Warnings
    extern bool warnDupeAddonNodes;
    extern bool warnRefHandleLimit;
    extern uint32_t warnRefrMainMenuLimit;
    extern uint32_t warnRefrLoadedGameLimit;

    // Experimental
    extern bool experimentalSaveGameMaxSize;
    extern bool experimentalTreatAllModsAsMasters;

    bool LoadConfig(const std::string& path);
}

#pragma once

namespace Settings
{
    namespace General
    {
        static REX::TOML::Bool bVerboseLogging("General", "bVerboseLogging", false);
        static REX::TOML::Bool bCleanSKSECoSaves("General", "bCleanSKSECoSaves", false);
    }

    namespace Fixes
    {
        static REX::TOML::Bool bArcheryDownwardAiming("Fixes", "bArcheryDownwardAiming", true);
        static REX::TOML::Bool bAnimationLoadSignedCrash("Fixes", "bAnimationLoadSignedCrash", true);
        static REX::TOML::Bool bBethesdaNetCrash("Fixes", "bBethesdaNetCrash", true);
        static REX::TOML::Bool bBGSKeywordFormLoadCrash("Fixes", "bBGSKeywordFormLoadCrash", true);
        static REX::TOML::Bool bBSLightingAmbientSpecular("Fixes", "bBSLightingAmbientSpecular", true);
        static REX::TOML::Bool bBSLightingShaderForceAlphaTest("Fixes", "bBSLightingShaderForceAlphaTest", true);
        static REX::TOML::Bool bBSLightingShaderParallaxBug("Fixes", "bBSLightingShaderParallaxBug", true);
        static REX::TOML::Bool bBSLightingShaderPropertyShadowMap("Fixes", "bBSLightingShaderPropertyShadowMap", true);
        static REX::TOML::Bool bBSTempEffectNiRTTI("Fixes", "bBSTempEffectNiRTTI", true);
        static REX::TOML::Bool bCalendarSkipping("Fixes", "bCalendarSkipping", true);
        static REX::TOML::Bool bCellInit("Fixes", "bCellInit", true);
        static REX::TOML::Bool bClimateLoad("Fixes", "bClimateLoad", true);
        static REX::TOML::Bool bConjurationEnchantAbsorbs("Fixes", "bConjurationEnchantAbsorbs", true);
        static REX::TOML::Bool bCreateArmorNodeNullPtrCrash("Fixes", "bCreateArmorNodeNullPtrCrash", true);
        static REX::TOML::Bool bDoublePerkApply("Fixes", "bDoublePerkApply", true);
        static REX::TOML::Bool bEquipShoutEventSpam("Fixes", "bEquipShoutEventSpam", true);
        static REX::TOML::Bool bESLCELLLoadBug("Fixes", "bESLCELLLoadBug", true);
        static REX::TOML::Bool bFaceGenMorphDataHeadNullPtrCrash("Fixes", "bFaceGenMorphDataHeadNullPtrCrash", true);
        static REX::TOML::Bool bGetKeywordItemCount("Fixes", "bGetKeywordItemCount", true);
        static REX::TOML::Bool bGHeapLeakDetectionCrash("Fixes", "bGHeapLeakDetectionCrash", true);
        static REX::TOML::Bool bGlobalTime("Fixes", "bGlobalTime", true);
        static REX::TOML::Bool bInitializeHitDataNullPtrCrash("Fixes", "bInitializeHitDataNullPtrCrash", true);
        static REX::TOML::Bool bLipSync("Fixes", "bLipSync", true);
        static REX::TOML::Bool bMemoryAccessErrors("Fixes", "bMemoryAccessErrors", true);
        static REX::TOML::Bool bMO5STypo("Fixes", "bMO5STypo", true);
        static REX::TOML::Bool bMusicOverlap("Fixes", "bMusicOverlap", true);
        static REX::TOML::Bool bNullProcessCrash("Fixes", "bNullProcessCrash", true);
        static REX::TOML::Bool bPerkFragmentIsRunning("Fixes", "bPerkFragmentIsRunning", true);
        static REX::TOML::Bool bPrecomputedPaths("Fixes", "bPrecomputedPaths", true);
        static REX::TOML::Bool bRemovedSpellBook("Fixes", "bRemovedSpellBook", true);
        static REX::TOML::Bool bSaveScreenshots("Fixes", "bSaveScreenshots", true);
        static REX::TOML::Bool bSavedHavokDataLoadInit("Fixes", "bSavedHavokDataLoadInit", true);
        static REX::TOML::Bool bShadowSceneNodeNullPtrCrash("Fixes", "bShadowSceneNodeNullPtrCrash", true);
        static REX::TOML::Bool bTextureLoadCrash("Fixes", "bTextureLoadCrash", true);
        static REX::TOML::Bool bTorchLandscape("Fixes", "bTorchLandscape", true);
        static REX::TOML::Bool bTreeReflections("Fixes", "bTreeReflections", true);
        static REX::TOML::Bool bVerticalLookSensitivity("Fixes", "bVerticalLookSensitivity", true);
        static REX::TOML::Bool bWeaponBlockScaling("Fixes", "bWeaponBlockScaling", true);
    }

    namespace Patches
    {
        static REX::TOML::Bool bDisableChargenPrecache("Patches", "bDisableChargenPrecache", false);
        static REX::TOML::Bool bDisableSnowFlag("Patches", "bDisableSnowFlag", false);
        static REX::TOML::Bool bEnableAchievementsWithMods("Patches", "bEnableAchievementsWithMods", true);
        static REX::TOML::Bool bFormCaching("Patches", "bFormCaching", true);
        static REX::TOML::Bool bINISettingCollection("Patches", "bINISettingCollection", true);
        static REX::TOML::Bool bMaxStdIO("Patches", "bMaxStdIO", true);
        static REX::TOML::Bool bRegularQuicksaves("Patches", "bRegularQuicksaves", false);
        static REX::TOML::Bool bSafeExit("Patches", "bSafeExit", true);
        static REX::TOML::Bool bSaveAddedSoundCategories("Patches", "bSaveAddedSoundCategories", true);
        static REX::TOML::I32  iSaveGameMaxSize("Patches", "iSaveGameMaxSize", 128);
        static REX::TOML::Bool bScrollingDoesntSwitchPOV("Patches", "bScrollingDoesntSwitchPOV", false);
        static REX::TOML::Bool bSleepWaitTime("Patches", "bSleepWaitTime", false);
        static REX::TOML::F32  fSleepWaitTimeModifier("Patches", "fSleepWaitTimeModifier", 0.3f);
        static REX::TOML::Bool bTreeLodReferenceCaching("Patches", "bTreeLodReferenceCaching", true);
        static REX::TOML::Bool bWaterflowAnimation("Patches", "bWaterflowAnimation", true);
        static REX::TOML::F32  fWaterflowSpeed("Patches", "fWaterflowSpeed", 20.0f);
    }

    namespace MemoryManager
    {
        static REX::TOML::Bool bOverrideMemoryManager("MemoryManager", "bOverrideMemoryManager", true);
        static REX::TOML::Bool bOverrideScrapHeap("MemoryManager", "bOverrideScrapHeap", true);
        static REX::TOML::Bool bOverrideScaleformAllocator("MemoryManager", "bOverrideScaleformAllocator", true);
        static REX::TOML::Bool bOverrideRenderPassCache("MemoryManager", "bOverrideRenderPassCache", true);
        static REX::TOML::Bool bReplaceImports("MemoryManager", "bReplaceImports", false);
    }

    namespace Warnings
    {
        static REX::TOML::Bool bTextureLoadFailed("Warnings", "bTextureLoadFailed", true);
        static REX::TOML::Bool bPrecomputedPathHasErrors("Warnings", "bPrecomputedPathHasErrors", true);
        static REX::TOML::Bool bRefHandleLimit("Warnings", "bRefHandleLimit", true);
        static REX::TOML::U32  uRefrMainMenuLimit("Warnings", "bRefrMainMenuLimit", 800000);
        static REX::TOML::U32  uRefrLoadedGameLimit("Warnings", "bRefrLoadedGameLimit", 1000000);
    }

    namespace Debug
    {
        static REX::TOML::Bool bPrintDetailedPrecomputedPathInfo("Debug", "bPrintDetailedPrecomputedPathInfo", false);
        static REX::TOML::Bool bDisableTBB("Debug", "bDisableTBB", false);
    }

    static void Load()
    {
        const auto toml = REX::TOML::SettingStore::GetSingleton();
        toml->Init(
            "Data/skse/plugins/EngineFixes.toml",
            "Data/skse/plugins/EngineFixesCustom.toml");
        toml->Load();
    }
}
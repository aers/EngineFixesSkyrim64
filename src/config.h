#pragma once

class config
{
private:
    using ISetting = AutoTOML::ISetting;

public:
    using bSetting = AutoTOML::bSetting;
    using fSetting = AutoTOML::fSetting;
    using iSetting = AutoTOML::iSetting;

    // EngineFixes
    static inline bSetting verboseLogging{ "EngineFixes", "VerboseLogging", false };
    static inline bSetting cleanSKSECosaves{ "EngineFixes", "CleanSKSECosaves", false };

    // Patches
    static inline bSetting patchDisableChargenPrecache{ "Patches", "DisableChargenPrecache", false };
    static inline bSetting patchEnableAchievementsWithMods{ "Patches", "EnableAchievementsWithMods", true };
    static inline bSetting patchFormCaching{ "Patches", "FormCaching", true };
    static inline iSetting patchMaxStdio{ "Patches", "MaxStdio", -1 };
    static inline bSetting patchMemoryManager{ "Patches", "MemoryManager", true };
    static inline bSetting patchRegularQuicksaves{ "Patches", "RegularQuicksaves", false };
    static inline bSetting patchSafeExit{ "Patches", "SafeExit", true };
    static inline bSetting patchSaveAddedSoundCategories{ "Patches", "SaveAddedSoundCategories", true };
    static inline bSetting patchScaleformAllocator{ "Patches", "ScaleformAllocator", true };
    static inline bSetting patchScrollingDoesntSwitchPOV{ "Patches", "ScrollingDoesntSwitchPOV", false };
    static inline bSetting patchSleepWaitTime{ "Patches", "SleepWaitTime", false };
    static inline fSetting sleepWaitTimeModifier{ "Patches", "SleepWaitTimeModifier", 0.3 };
    static inline bSetting patchTreeLODReferenceCaching{ "Patches", "TreeLODReferenceCaching", true };
    static inline bSetting patchWaterflowAnimation{ "Patches", "WaterflowAnimation", true };
    static inline fSetting waterflowSpeed{ "Patches", "WaterflowSpeed", 20.0 };

    // Fixes
    static inline bSetting fixAnimationLoadSignedCrash{ "Fixes", "AnimationLoadSignedCrash", true };
    static inline bSetting fixArcheryDownwardAiming{ "Fixes", "ArcheryDownwardAiming", true };
    static inline bSetting fixBethesdaNetCrash{ "Fixes", "BethesdaNetCrash", true };
    static inline bSetting fixBSLightingAmbientSpecular{ "Fixes", "BSLightingAmbientSpecular", true };
    static inline bSetting fixBSLightingShaderForceAlphaTest{ "Fixes", "BSLightingShaderForceAlphaTest", true };
    static inline bSetting fixBSLightingShaderGeometryParallaxBug{ "Fixes", "BSLightingShaderParallaxBug", true };
    static inline bSetting fixBSTempEffectNiRTTI{ "Fixes", "BSTempEffectNiRTTI", true };
    static inline bSetting fixCalendarSkipping{ "Fixes", "CalendarSkipping", true };
    static inline bSetting fixCellInit{ "Fixes", "CellInit", true };
    static inline bSetting fixCreateArmorNodeNullptrCrash{ "Fixes", "CreateArmorNodeNullptrCrash", true };
    static inline bSetting fixConjurationEnchantAbsorbs{ "Fixes", "ConjurationEnchantAbsorbs", true };
    static inline bSetting fixDoublePerkApply{ "Fixes", "DoublePerkApply", true };
    static inline bSetting fixEquipShoutEventSpam{ "Fixes", "EquipShoutEventSpam", true };
    static inline bSetting fixFaceGenMorphDataHeadNullptrCrash{ "Fixes", "FaceGenMorphDataHeadNullptrCrash", true };
    static inline bSetting fixGetKeywordItemCount{ "Fixes", "GetKeywordItemCount", true };
    static inline bSetting fixGHeapLeakDetectionCrash{ "Fixes", "GHeapLeakDetectionCrash", true };
    static inline bSetting fixGlobalTime{ "Fixes", "GlobalTime", true };
    static inline bSetting fixInitializeHitDataNullptrCrash{ "Fixes", "InitializeHitDataNullptrCrash", true };
    static inline bSetting fixLipSync{ "Fixes", "LipSync", true };
    static inline bSetting fixMemoryAccessErrors{ "Fixes", "MemoryAccessErrors", true };
    static inline bSetting fixMusicOverlap{ "Fixes", "MusicOverlap", true };
    static inline bSetting fixMO5STypo{ "Fixes", "MO5STypo", true };
    static inline bSetting fixNullProcessCrash{ "Fixes", "NullProcessCrash", true };
    static inline bSetting fixPerkFragmentIsRunning{ "Fixes", "PerkFragmentIsRunning", true };
    static inline bSetting fixRemovedSpellBook{ "Fixes", "RemovedSpellBook", true };
    static inline bSetting fixSaveScreenshots{ "Fixes", "SaveScreenshots", true };
    static inline bSetting fixShadowSceneNodeNullptrCrash{ "Fixes", "ShadowSceneNodeNullptrCrash", true };
    static inline bSetting fixSlowTimeCameraMovement{ "Fixes", "SlowTimeCameraMovement", true };
    static inline bSetting fixTorchLandscape{ "Fixes", "TorchLandscape", true };
    static inline bSetting fixTreeReflections{ "Fixes", "TreeReflections", true };
    static inline bSetting fixVerticalLookSensitivity{ "Fixes", "VerticalLookSensitivity", true };
    static inline bSetting fixWeaponBlockScaling{ "Fixes", "WeaponBlockScaling", true };

    // Warnings
    static inline bSetting warnDupeAddonNodes{ "Warnings", "DupeAddonNodes", false };
    static inline bSetting warnRefHandleLimit{ "Warnings", "RefHandleLimit", true };
    static inline iSetting warnRefrMainMenuLimit{ "Warnings", "RefrMainMenuLimit", 800000 };
    static inline iSetting warnRefrLoadedGameLimit{ "Warnings", "RefrLoadedGameLimit", 1000000 };

    // Experimental
    static inline bSetting experimentalSaveGameMaxSize{ "Experimental", "SaveGameMaxSize", false };
    static inline bSetting experimentalTreatAllModsAsMasters{ "Experimental", "TreatAllModsAsMasters", false };

    static bool load_config(const std::string& a_path)
    {
        try
        {
            const auto table = toml::parse_file(a_path);
            const auto& settings = ISetting::get_settings();
            for (const auto& setting : settings)
            {
                try
                {
                    setting->load(table);
                } catch (const std::exception& e)
                {
                    logger::warn(e.what());
                }
            }
        } catch (const toml::parse_error& e)
        {
            std::ostringstream ss;
            ss
                << "Error parsing file \'" << *e.source().path
                << "\':\n"
                << e.description()
                << "\n  (" << e.source().begin << ")\n";
            logger::error(ss.str());
            return false;
        }

        return true;
    }
};

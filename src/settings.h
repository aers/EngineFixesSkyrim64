#pragma once

namespace Settings
{
    namespace General
    {
        static REX::INI::Bool bVerboseLogging("General", "bVerboseLogging", false);
        static REX::INI::Bool bCleanSKSECoSaves("General", "bCleanSKSECoSaves", false);
    }

    namespace Fixes
    {
        static REX::INI::Bool bArcheryDownwardAiming("Fixes", "bArcheryDownwardAiming", true);
        static REX::INI::Bool bAnimationLoadSignedCrash("Fixes", "bAnimationLoadSignedCrash", true);
        static REX::INI::Bool bBethesdaNetCrash("Fixes", "bBethesdaNetCrash", true);
        static REX::INI::Bool bBSLightingAmbientSpecular("Fixes", "bBSLightingAmbientSpecular", true);
        static REX::INI::Bool bBSLightingShaderForceAlphaTest("Fixes", "bBSLightingShaderForceAlphaTest", true);
        static REX::INI::Bool bBSLightingShaderParallaxBug("Fixes", "bBSLightingShaderParallaxBug", true);
        static REX::INI::Bool bMemoryAccessErrors("Fixes", "bMemoryAccessErrors", true);
    }

    namespace Patches
    {
        static REX::INI::Bool bDisableChargenPrecache("Patches", "DisableChargenPrecache", false);
        static REX::INI::Bool bEnableAchievementsWithMods("Patches", "EnableAchievementsWithMods", true);
        static REX::INI::Bool bFormCaching("Patches", "bFormCaching", true);
        static REX::INI::I32 iMaxStdIO("Patches", "bMaxStdIO", 8192);
        static REX::INI::Bool bMemoryManager("Patches", "bMemoryManager", true);
        static REX::INI::Bool bRegularQuicksaves("Patches", "bRegularQuicksaves", false);
        static REX::INI::Bool bSafeExit("Patches", "bSafeExit", true);
        static REX::INI::Bool bSaveAddedSoundCategories("Patches", "bSaveAddedSoundCategories", true);
        static REX::INI::Bool bScrollingDoesntSwitchPOV("Patches", "bScrollingDoesntSwitchPOV", false);
        static REX::INI::Bool bSleepWaitTime("Patches", "bSleepWaitTime", false);
        static REX::INI::F32 fSleepWaitTimeModifier("Patches", "bSleepWaitTimeModifier", 0.3f);
        static REX::INI::Bool bTreeLodReferenceCaching("Patches", "bTreeLodReferenceCaching", true);
        static REX::INI::Bool bWaterflowAnimation("Patches", "bWaterflowAnimation", true);
        static REX::INI::F32 fWaterflowSpeed("Patches", "bWaterflowSpeed", 20.0f);
    }

	namespace MemoryManager
    {
		static REX::INI::Bool bOverrideCRTAllocator( "MemoryManager", "bOverrideCRTAllocator", true);
		static REX::INI::Bool bOverrideGlobalMemoryManager( "MemoryManager", "bOverrideGlobalMemoryManager", true);
        static REX::INI::Bool bOverrideScrapHeap("MemoryManager", "bOverrideScrapHeap", true);
        static REX::INI::Bool bOverrideScaleformAllocator( "MemoryManager", "bOverrideScaleformAllocator", true);
	}

	static void Load()
	{
		const auto ini = REX::INI::SettingStore::GetSingleton();
		ini->Init(
			"Data/skse/plugins/EngineFixes.ini",
			"Data/skse/plugins/EngineFixesCustom.ini");
		ini->Load();
	}
}
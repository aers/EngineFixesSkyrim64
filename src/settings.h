#pragma once

namespace Settings
{
    namespace Patches
    {
        static REX::INI::Bool bDisableChargenPrecache("Patches", "DisableChargenPrecache", false);
        static REX::INI::Bool bEnableAchievementsWithMods("Patches", "EnableAchievementsWithMods", true);
        static REX::INI::Bool bFormCaching("Patches", "bFormCaching", true);
        static REX::INI::I32 bMaxStdIO("Patches", "bMaxStdIO", 8192);
        static REX::INI::Bool bRegularQuicksaves("Patches", "bRegularQuicksaves", false);
        static REX::INI::Bool bSafeExit("Patches", "bSafeExit", true);
        static REX::INI::Bool bSaveAddedSoundCategories("Patches", "bSaveAddedSoundCategories", true);
        static REX::INI::Bool bTreeLodReferenceCaching("Patches", "bTreeLodReferenceCaching", true);
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
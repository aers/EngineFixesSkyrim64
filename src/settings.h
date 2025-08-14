#pragma once

namespace Settings
{
    namespace Patches
    {
        static REX::INI::Bool bFormCaching("Patches", "bFormCaching", true);
    }

	namespace MemoryManager
    {
		static REX::INI::Bool bOverrideCRTAllocator( "MemoryManager", "bOverrideCRTAllocator", true );
		static REX::INI::Bool bOverrideGlobalMemoryManager( "MemoryManager", "bOverrideGlobalMemoryManager", true );
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
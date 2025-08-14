#pragma once

namespace Settings
{
	namespace MemoryManager {
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
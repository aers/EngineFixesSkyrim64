#include "../skse64_loader_common/IdentifyEXE.h"
#include "../skse64/PluginAPI.h"
#include "../skse64_common/skse_version.h"
#include "../skse64_common/BranchTrampoline.h"

#include <ShlObj.h>

#include <jemalloc/jemalloc.h>

#include "config.h"
#include "patches.h"
#include "util.h"
#include <cinttypes>

IDebugLog	gLog;
bool		preloaded = false;

PluginHandle					g_pluginHandle = kPluginHandle_Invalid;
SKSEMessagingInterface			* g_messaging = nullptr;

void TempFixSKEE()
{
	const auto skee = reinterpret_cast<uintptr_t>(GetModuleHandle("skee64"));
	if (skee)
	{
		_MESSAGE("skee found, enabling temporary patch to fix XPMSSE 4.3 issue");
		if (*(uint32_t *)(skee + 0x6C10) != 0x0FD28548)
		{
			_MESSAGE("unknown skee version, canceling");
		}
		else
		{
			SafeWrite8(skee + 0x6C10, 0xC3);
			_MESSAGE("patched");
		}
	}
}

void SKSEMessageHandler(SKSEMessagingInterface::Message * message)
{
	switch (message->type)
	{
	case SKSEMessagingInterface::kMessage_DataLoaded:
		{
			TempFixSKEE();
			if (config::patchSaveAddedSoundCategories)
				SaveAddedSoundCategories::LoadVolumes();
		}
		break;
	default: 
		break;
	}
}

void SetupLog()
{
	gLog.OpenRelative(CSIDL_MYDOCUMENTS, R"(\My Games\Skyrim Special Edition\SKSE\EngineFixes64.log)");
#ifdef _DEBUG
	gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);
#else
	gLog.SetLogLevel(IDebugLog::kLevel_Message);
#endif		

	_MESSAGE("EngineFixes64");
}

void LoadConfig()
{
	const std::string& runtimePath = GetRuntimeDirectory();

	if (config::LoadConfig(runtimePath + R"(Data\SKSE\plugins\EngineFixes64.ini)"))
		_MESSAGE("loaded configuration successfully");
	else
		_MESSAGE("using default config");
}

bool SetupTrampolines()
{
	if (!g_branchTrampoline.Create(1024 * 64))
	{
		_FATALERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
		return false;
	}

	if (!g_localTrampoline.Create(1024 * 64, nullptr))
	{
		_FATALERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
		return false;
	}

	return true;
}

extern "C" {
	// preloader
	void Initialize()
	{
		SetupLog();
		// check version
		// can't use the skse plugin interface for obvious reasons
		{
			TCHAR filename[MAX_PATH];
			GetModuleFileName(nullptr, filename, MAX_PATH);

			VS_FIXEDFILEINFO info;
			std::string productName;
			std::string productVersion;

			if (GetFileVersion(filename, &info, &productName, &productVersion))
			{
				uint64_t version;

				VersionStrToInt(productVersion, &version);

				const uint32_t skse_internal_version = MAKE_EXE_VERSION(version >> 48, version >> 32, version >> 16);

				if (skse_internal_version != RUNTIME_VERSION_1_5_39)
				{
					_FATALERROR("unsupported runtime version %08X", version);
					return;
				}
			}
		}	

		preloaded = true;
		LoadConfig();

		_MESSAGE("plugin was preloaded, allowing preload-required features");

		if (!SetupTrampolines())
			return;

		if (config::patchMemoryManager)
		{
			if (config::patchSnowSparkle)
				MemoryManager::Patch();
			else
				_MESSAGE("memory manager patch enabled but snow sparkle patch is not, will not patch due to game crashes");
		}

		if (config::patchBSReadWriteLock)
			BSReadWriteLockCustom::Patch();

	}

	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		if (!preloaded)
			SetupLog();

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "EngineFixes64 plugin";
		info->version = 1;

		g_pluginHandle = skse->GetPluginHandle();


		if (skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");
			return false;
		}
		else if (skse->runtimeVersion != RUNTIME_VERSION_1_5_39)
		{
			_FATALERROR("unsupported runtime version %08X", skse->runtimeVersion);
			return false;
		}

		g_messaging = (SKSEMessagingInterface *)skse->QueryInterface(kInterface_Messaging);
		if (!g_messaging) {
			_ERROR("couldn't get messaging interface, disabling patches that require it");
		}

		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse) 
	{
		if (!preloaded)
		{
			LoadConfig();
			if (!SetupTrampolines())
				return false;
		}

		if (g_messaging)
			g_messaging->RegisterListener(g_pluginHandle, "SKSE", SKSEMessageHandler);
		
		if (config::patchFormCaching)
			FormCaching::Patch();

		if (config::patchDoublePerkApply)
			DoublePerkApply::Patch();

		if (config::patchSlowTimeCameraMovement)
			SlowTimeCameraMove::Patch();

		if (config::patchVerticalLookSensitivity)
			VerticalLookSensitivity::Patch();

		if (config::patchWaterflowTimer)
			WaterflowTimer::Patch();

		if (config::patchTreeReflections)
			TreeReflection::Patch();

		if (config::patchSnowSparkle)
			SnowSparkle::Patch();

		if (config::patchSaveAddedSoundCategories && g_messaging)
			SaveAddedSoundCategories::Patch();

		_MESSAGE("all patches applied");

		return true;
	}
};
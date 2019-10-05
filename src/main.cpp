#include "skse64/PluginAPI.h" 

#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/skse_version.h"
#include "skse64_common/Utilities.h"

#include "SKSE/API.h"

#include <sstream>
#include <ShlObj.h>

#include "version.h"
#include "config.h"
#include "fixes.h"
#include "patches.h"
#include "utils.h"
#include "warnings.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type)
    {
	case SKSE::MessagingInterface::kDataLoaded:
    {
		_MESSAGE("beginning post-load patches");
        // patch post load so ini settings are loaded
        if (config::fixSaveScreenshots)
            fixes::PatchSaveScreenshots();

        if (config::warnRefHandleLimit)
        {
            warnings::WarnActiveRefrHandleCount(config::warnRefrMainMenuLimit);
        }

        if (config::patchSaveAddedSoundCategories)
            patches::LoadVolumes();

        // temporary fix for SKSE crosshair ref event dispatch

        _VMESSAGE("temporary fix for SKSE crosshair ref event dispatch");

        const auto handle = (uintptr_t) GetModuleHandleA("skse64_1_5_80");

        if (handle && *(uint8_t *)(handle+0xD658) == 0x4D)
        {
            _VMESSAGE("skse 2.0.16 found");
            constexpr uintptr_t START = 0xD658;
            constexpr uintptr_t END = 0xD661;
            constexpr UInt8 NOP = 0x90;

            // .text:000000018000D658                 test    r8, r8
            // .text:000000018000D65B                 jz      loc_18000D704

            for (uintptr_t i = START; i < END; ++i) {
                SafeWrite8(handle + i, NOP);
            }
        }

        _VMESSAGE("clearing node map");
        warnings::ClearNodeMap();

		_MESSAGE("post-load patches complete");
    }
    break;
	case SKSE::MessagingInterface::kPostLoadGame:
        {
        if (config::warnRefHandleLimit)
        {
            warnings::WarnActiveRefrHandleCount(config::warnRefrLoadedGameLimit);
        }
        }
    break;
    default:
        break;
    }
}

extern "C" {
	bool SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
    {
		SKSE::Logger::OpenRelative(FOLDERID_Documents, R"(\My Games\Skyrim Special Edition\SKSE\EngineFixes.log)");
#ifdef _DEBUG
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);
#else
		SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kMessage);
		SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kMessage);
#endif	
		SKSE::Logger::UseLogStamp(true);

		_MESSAGE("Engine Fixes v%s", EF_VERSION_VERSTRING);

        // populate info structure
        a_info->infoVersion = SKSE::PluginInfo::kVersion;
        a_info->name = "EngineFixes plugin";
		a_info->version = EF_VERSION_MAJOR;

        if (a_skse->IsEditor())
        {
            _FATALERROR("loaded in editor, marking as incompatible");
            return false;
        }

		switch (a_skse->RuntimeVersion()) {
		case RUNTIME_VERSION_1_5_73:
		case RUNTIME_VERSION_1_5_80:
			break;
		default:
			_FATALERROR("Unsupported runtime version %08X!\n", a_skse->RuntimeVersion());
			return false;
		}

        return true;
    }

	bool SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
    {
		if (!SKSE::Init(a_skse)) {
			return false;
		}


		if (!SKSE::AllocLocalTrampoline(1024 * 32) || !SKSE::AllocBranchTrampoline(1024 * 32)) {
			return false;
		}

		auto messaging = SKSE::GetMessagingInterface();
		if (messaging->RegisterListener("SKSE", MessageHandler)) {
			_MESSAGE("Messaging interface registration successful");
		}
		else {
			_FATALERROR("Messaging interface registration failed!\n");
			return false;
		}

		const auto runtimePath = GetRuntimeDirectory();

		if (config::LoadConfig(runtimePath + R"(Data\SKSE\plugins\EngineFixes.ini)"))
		{
			_MESSAGE("loaded config successfully");
		}
		else
		{
			_MESSAGE("config load failed, using default config");
		}

		_MESSAGE("beginning pre-load patches");

		if (config::cleanSKSECosaves)
			CleanSKSECosaves();

		patches::PatchAll();
		fixes::PatchAll();
		warnings::PatchAll();
		
		_MESSAGE("pre-load patches complete");
        return true;
    }
}

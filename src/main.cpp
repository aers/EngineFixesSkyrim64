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

		if (config::fixTreeReflections)
			fixes::PatchTreeReflections();

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
		case RUNTIME_VERSION_1_5_97:
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


		if (!SKSE::AllocLocalTrampoline(1024 * 2) || !SKSE::AllocBranchTrampoline(1024 * 2)) {
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

		if (config::verboseLogging)
		{
			_MESSAGE("enabling verbose logging");
			SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kVerboseMessage);
			SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kVerboseMessage);
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

#include "skse64/PluginAPI.h" 

#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/skse_version.h"
#include "skse64_common/Utilities.h"

#include <sstream>
#include <ShlObj.h>

#include "config.h"
#include "fixes.h"
#include "patches.h"
#include "utils.h"
#include "warnings.h"


bool		preloaded = false;

PluginHandle					g_pluginHandle = kPluginHandle_Invalid;
SKSEMessagingInterface			* g_messaging = nullptr;

void SKSEMessageHandler(SKSEMessagingInterface::Message * message)
{
    switch (message->type)
    {
    case SKSEMessagingInterface::kMessage_DataLoaded:
    {
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

        const auto handle = (uintptr_t) GetModuleHandleA("skse64_1_5_73");

        if (handle && *(uint8_t *)(handle+0xD658) == 0x4D)
        {
            _MESSAGE("skse 2.0.15 found");
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
    }
    break;
    case SKSEMessagingInterface::kMessage_PostLoadGame:
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
    void Initialize()
    {

        IDebugLog::OpenRelative(CSIDL_MYDOCUMENTS, R"(\My Games\Skyrim Special Edition\SKSE\EngineFixes.log)");
#ifdef _DEBUG
        IDebugLog::SetLogLevel(IDebugLog::kLevel_DebugMessage);
#else
        IDebugLog::SetLogLevel(IDebugLog::kLevel_Message);
#endif		

        _MESSAGE("EngineFixes for Skyrim Special Edition");

        // check version

        const auto version = GetGameVersion();

        if (version != RUNTIME_VERSION_1_5_73)
        {
            _FATALERROR("unsupported runtime version %08X", version);
            return;
        }

        if (!g_branchTrampoline.Create(1024 * 64))
        {
            _FATALERROR("couldn't create branch trampoline. this is fatal. skipping remainder of init process.");
            return;
        }

        if (!g_localTrampoline.Create(1024 * 64, nullptr))
        {
            _FATALERROR("couldn't create codegen buffer. this is fatal. skipping remainder of init process.");
            return;
        }


        _MESSAGE("plugin preloaded successfully");
        preloaded = true;

        const auto runtimePath = GetRuntimeDirectory();

        if (config::LoadConfig(runtimePath + R"(Data\SKSE\plugins\EngineFixes.ini)"))
        {
            _MESSAGE("loaded config successfully");
        }
        else
        {
            _MESSAGE("config load failed, using default config");
        }

#ifndef _DEBUG
        if (config::verboseLogging)
            IDebugLog::SetLogLevel(IDebugLog::kLevel_VerboseMessage);
#endif

        _MESSAGE("patching game");
        if (config::cleanSKSECosaves)
            CleanSKSECosaves();

        patches::PatchAll();
        fixes::PatchAll();
        warnings::PatchAll();
    }

    bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
    {
        // populate info structure
        info->infoVersion = PluginInfo::kInfoVersion;
        info->name = "EngineFixes plugin";
        info->version = 3;

        g_pluginHandle = skse->GetPluginHandle();

        if (!preloaded)
        {
            IDebugLog::OpenRelative(CSIDL_MYDOCUMENTS, R"(\My Games\Skyrim Special Edition\SKSE\EngineFixes.log)");
#ifdef _DEBUG
            IDebugLog::SetLogLevel(IDebugLog::kLevel_DebugMessage);
#else
            IDebugLog::SetLogLevel(IDebugLog::kLevel_Message);
#endif		
            _FATALERROR("plugin was not preloaded, please read the installation instructions carefully");
            return false;
        }

        if (skse->isEditor)
        {
            _MESSAGE("loaded in editor, marking as incompatible");
            return false;
        }
        
        if (skse->runtimeVersion != RUNTIME_VERSION_1_5_73)
        {
            _FATALERROR("unsupported runtime version %08X", skse->runtimeVersion);
            return false;
        }



        g_messaging = static_cast<SKSEMessagingInterface *>(skse->QueryInterface(kInterface_Messaging));
        if (!g_messaging) {
            _ERROR("couldn't get messaging interface, disabling patches that require it");
        }

        return true;

    }

    bool SKSEPlugin_Load(const SKSEInterface * skse)
    {
        if (g_messaging)
            g_messaging->RegisterListener(g_pluginHandle, "SKSE", SKSEMessageHandler);

        return true;
    }
}

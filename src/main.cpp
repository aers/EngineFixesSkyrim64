#include "config.h"
#include "fixes.h"
#include "patches.h"
#include "utils.h"
#include "version.h"
#include "warnings.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type)
    {
    case SKSE::MessagingInterface::kDataLoaded:
        {
            _MESSAGE("beginning post-load patches");
            if (*config::cleanSKSECosaves)
                CleanSKSECosaves();

            // patch post load so ini settings are loaded
            if (*config::fixSaveScreenshots)
                fixes::PatchSaveScreenshots();

            if (*config::warnRefHandleLimit)
            {
                warnings::WarnActiveRefrHandleCount(*config::warnRefrMainMenuLimit);
            }

            if (*config::patchSaveAddedSoundCategories)
                patches::LoadVolumes();

            if (*config::fixTreeReflections)
                fixes::PatchTreeReflections();

            _VMESSAGE("clearing node map");
            warnings::ClearNodeMap();

            _MESSAGE("post-load patches complete");
        }
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
        {
            if (*config::warnRefHandleLimit)
            {
                warnings::WarnActiveRefrHandleCount(*config::warnRefrLoadedGameLimit);
            }
        }
        break;
    default:
        break;
    }
}

bool CheckVersion(const SKSE::Version& a_version)
{
    auto success = a_version >= SKSE::RUNTIME_1_5_39;
    if (!success)
    {
        _FATALERROR("Unsupported runtime version %s!\n", a_version.GetString().c_str());
    }
    return success;
}

extern "C" void DLLEXPORT APIENTRY Initialize()
{
#ifdef _DEBUG
    while (!IsDebuggerPresent())
    {
    }
#endif

    SKSE::Logger::OpenRelative(FOLDERID_Documents, "My Games/Skyrim Special Edition/SKSE/EngineFixes.log");
#ifdef _DEBUG
    SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kDebugMessage);
    SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kDebugMessage);
    SKSE::Logger::TrackTrampolineStats(true);
#else
    SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kMessage);
    SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kMessage);
#endif
    SKSE::Logger::UseLogStamp(true);

    _MESSAGE("Engine Fixes v%s", EF_VERSION_VERSTRING);

    if (config::load_config("Data/SKSE/Plugins/EngineFixes.toml"))
    {
        _MESSAGE("loaded config successfully");
    }
    else
    {
        _MESSAGE("config load failed, using default config");
    }

    auto ver = REL::Module::GetVersion();
    if (!CheckVersion(ver))
    {
        return;
    }

    if (!SKSE::AllocTrampoline(1 << 11))
    {
        return;
    }

    patches::Preload();
}

extern "C" bool DLLEXPORT APIENTRY SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    // populate info structure
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "EngineFixes plugin";
    a_info->version = EF_VERSION_MAJOR;

    if (a_skse->IsEditor())
    {
        _FATALERROR("loaded in editor, marking as incompatible");
        return false;
    }

    auto ver = a_skse->RuntimeVersion();
    if (!CheckVersion(ver))
    {
        return false;
    }

    return true;
}

extern "C" bool DLLEXPORT APIENTRY SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    if (!SKSE::Init(a_skse))
    {
        return false;
    }

    auto trampoline = SKSE::GetTrampoline();
    if (trampoline->Empty())
    {
        return false;
    }

    auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler))
    {
        return false;
    }

    if (*config::verboseLogging)
    {
        _MESSAGE("enabling verbose logging");
        SKSE::Logger::SetPrintLevel(SKSE::Logger::Level::kVerboseMessage);
        SKSE::Logger::SetFlushLevel(SKSE::Logger::Level::kVerboseMessage);
    }

    _MESSAGE("beginning pre-load patches");

    patches::PatchAll();
    fixes::PatchAll();
    warnings::PatchAll();

    _MESSAGE("pre-load patches complete");
    return true;
}

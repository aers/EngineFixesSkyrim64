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
        logger::info("beginning post-load patches"sv);
        if (*config::cleanSKSECosaves)
            CleanSKSECosaves();

        // patch post load so ini settings are loaded
        if (*config::fixSaveScreenshots)
            fixes::PatchSaveScreenshots();

        if (*config::warnRefHandleLimit)
            warnings::WarnActiveRefrHandleCount(static_cast<std::uint32_t>(*config::warnRefrMainMenuLimit));

        if (*config::patchSaveAddedSoundCategories)
            patches::LoadVolumes();

        if (*config::fixTreeReflections)
            fixes::PatchTreeReflections();

        logger::trace("clearing node map"sv);
        warnings::ClearNodeMap();

        logger::info("post-load patches complete"sv);

        break;
    case SKSE::MessagingInterface::kPostLoadGame:
        if (*config::warnRefHandleLimit)
            warnings::WarnActiveRefrHandleCount(static_cast<std::uint32_t>(*config::warnRefrLoadedGameLimit));

        break;
    default:
        break;
    }
}

bool CheckVersion(const REL::Version& a_version)
{
    const auto success = a_version >= SKSE::RUNTIME_1_5_39;
    if (!success)
        logger::critical("Unsupported runtime version {}"sv, a_version.string());

    return success;
}

extern "C" void DLLEXPORT APIENTRY Initialize()
{
#ifdef _DEBUG
    while (!IsDebuggerPresent())
    {
    }
#endif

#ifndef NDEBUG
    auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
    auto path = logger::log_directory();
    if (!path)
        stl::report_and_fail("failed to get standard log path"sv);

    *path /= "EngineFixes.log"sv;
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

    auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
    log->set_level(spdlog::level::trace);
#else
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::warn);
#endif

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

    logger::info("Engine Fixes v{}"sv, Version::MAJOR);

    if (config::load_config("Data/SKSE/Plugins/EngineFixes.toml"s))
        logger::info("loaded config successfully"sv);
    else
        logger::warn("config load failed, using default config"sv);

    const auto ver = REL::Module::get().version();
    if (!CheckVersion(ver))
        return;

    SKSE::AllocTrampoline(1 << 11);

    patches::Preload();
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
    // populate info structure
    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = "EngineFixes plugin";
    a_info->version = Version::MAJOR;

    if (a_skse->IsEditor())
    {
        logger::critical("loaded in editor, marking as incompatible"sv);
        return false;
    }

    const auto ver = a_skse->RuntimeVersion();
    if (!CheckVersion(ver))
        return false;

    return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler))
        return false;

    if (*config::verboseLogging)
    {
        logger::info("enabling verbose logging"sv);
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);
    }

    logger::info("beginning pre-load patches"sv);

    patches::PatchAll();
    fixes::PatchAll();
    warnings::PatchAll();

    logger::info("pre-load patches complete"sv);

    return true;
}

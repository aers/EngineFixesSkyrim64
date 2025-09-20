#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

#include "clean_cosaves.h"
#include "fixes/bslightingshader_parallax_bug.h"
#include "fixes/fixes.h"
#include "fixes/save_screenshots.h"
#include "fixes/tree_reflections.h"
#include "memory/allocator.h"
#include "memory/memory.h"
#include "patches/patches.h"
#include "patches/save_added_sound_categories.h"
#include "settings.h"
#include "warnings/warnings.h"

bool g_isPreloaded = false;

std::chrono::high_resolution_clock::time_point start;

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded:
        {
            if (Settings::General::bCleanSKSECoSaves.GetValue())
                Util::CoSaves::Clean();
            if (Settings::Patches::bSaveAddedSoundCategories.GetValue())
                Patches::SaveAddedSoundCategories::LoadVolumes();
            // need ini settings
            if (Settings::Fixes::bSaveScreenshots.GetValue())
                Fixes::SaveScreenshots::Install();
            // need to make sure enb dll has loaded
            if (Settings::Fixes::bTreeReflections.GetValue())
                Fixes::TreeReflections::Install();
            // need to detect community shaders
            if (Settings::Fixes::bBSLightingShaderParallaxBug.GetValue())
                Fixes::BSLightingShaderParallaxBug::Install();
            if (Settings::Warnings::bRefHandleLimit.GetValue()) {
                Warnings::WarnActiveRefrHandleCount(Settings::Warnings::uRefrMainMenuLimit.GetValue());
            }

            auto timeElapsed = std::chrono::high_resolution_clock::now() - start;
            logger::info("time to main menu {}"sv, std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed).count());

            break;
        }
    case SKSE::MessagingInterface::kPostLoadGame:
        {
            if (Settings::Warnings::bRefHandleLimit.GetValue()) {
                Warnings::WarnActiveRefrHandleCount(Settings::Warnings::uRefrLoadedGameLimit.GetValue());
            }
        }
    default:
        break;
    }
}

void OpenLog()
{
    auto path = SKSE::log::log_directory();

    if (!path)
        return;

    *path /= "EngineFixes.log";

    std::vector<spdlog::sink_ptr> sinks{
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true),
        std::make_shared<spdlog::sinks::msvc_sink_mt>()
    };

    auto logger = std::make_shared<spdlog::logger>("global", sinks.begin(), sinks.end());

#ifndef NDEBUG
    logger->set_level(spdlog::level::debug);
    logger->flush_on(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);
#endif

    spdlog::set_default_logger(std::move(logger));
    spdlog::set_pattern("[%Y-%m-%d %T.%e][%-16s:%-4#][%L]: %v");
}

extern "C" __declspec(dllexport) void __stdcall Initialize()
{
    start = std::chrono::high_resolution_clock::now();
    OpenLog();

    logger::info("EngineFixes v{}.{}.{} PreLoad"sv, Version::MAJOR, Version::MINOR, Version::PATCH);

    const auto ver = REL::Module::get().version();
    if (ver < VAR_NUM(SKSE::RUNTIME_SSE_1_5_97, SKSE::RUNTIME_SSE_1_6_1170)) {
        logger::error("Unsupported runtime version {}"sv, ver);
        return;
    }

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(1 << 11);

    Settings::Load();

    if (Settings::General::bVerboseLogging.GetValue()) {
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);

        logger::trace("enabled verbose logging"sv);
    }

    Memory::Install();
    Patches::Install();
    Fixes::Install();

    g_isPreloaded = true;
}

#ifdef SKYRIM_AE
extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = []() {
    SKSE::PluginVersionData v;
    v.PluginVersion(Version::MAJOR);
    v.PluginName(Version::PROJECT);
    v.AuthorName("aers");
    v.UsesAddressLibrary();
    v.UsesUpdatedStructs();
    v.CompatibleVersions({ SKSE::RUNTIME_SSE_1_6_1170 });

    return v;
}();
#else
extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* a_info)
{
    if (!g_isPreloaded) {
        OpenLog();
        logger::error("plugin did not preload, please install the preloader");

        std::wostringstream messageBoxText;
        messageBoxText << L"ERROR: Engine Fixes did not pre-load and fixes are not active. Please verify the installation of d3dx9_42.dll from the Part 2 archive. This file must reside in the main game folder alongside SkyrimSE.exe, or be properly installed with your mod manager's root folder functionality.\r\n"sv;
        messageBoxText << L"Skyrim will now close.";
        REX::W32::MessageBoxW(nullptr, messageBoxText.str().c_str(), L"Engine Fixes", MB_OK);

        spdlog::default_logger()->flush();
        ::TerminateProcess(::GetCurrentProcess(), EXIT_SUCCESS);

        return false;
    }

    a_info->infoVersion = SKSE::PluginInfo::kVersion;
    a_info->name = Version::PROJECT.data();
    a_info->version = Version::MAJOR;

    return true;
}
#endif

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    if (!g_isPreloaded) {
        // init with log
        SKSE::Init(a_skse);

        logger::error("plugin did not preload, please install the preloader");

        std::wostringstream messageBoxText;
        messageBoxText << L"ERROR: Engine Fixes did not pre-load and fixes are not active. Please verify the installation of d3dx9_42.dll from the Part 2 archive. This file must reside in the main game folder alongside SkyrimSE.exe, or be properly installed with your mod manager's root folder functionality.\r\n"sv;
        messageBoxText << L"Skyrim will now close.";
        REX::W32::MessageBoxW(nullptr, messageBoxText.str().c_str(), L"Engine Fixes", MB_OK);

        spdlog::default_logger()->flush();
        ::TerminateProcess(::GetCurrentProcess(), EXIT_SUCCESS);

        return false;
    }

    SKSE::Init(a_skse, false);

    logger::info("EngineFixes SKSE Load"sv, Version::PROJECT);

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler)) {
        logger::error("Failed to register messaging interface listener"sv);
        return false;
    }

    return true;
}

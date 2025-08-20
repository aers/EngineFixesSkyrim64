#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

#include "Patches/patches.h"
#include "Patches/save_added_sound_categories.h"
#include "clean_cosaves.h"
#include "fixes/fixes.h"
#include "fixes/save_screenshots.h"
#include "fixes/tree_reflections.h"
#include "settings.h"
#include "warnings/warnings.h"

bool g_isPreloaded = false;

std::chrono::high_resolution_clock::time_point start;

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type) {
    case SKSE::MessagingInterface::kDataLoaded: {
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

void OpenLog() {
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

extern "C" __declspec(dllexport) void __stdcall Initialize() {
    start = std::chrono::high_resolution_clock::now();
	OpenLog();

    logger::info("EngineFixes v{}.{}.{} PreLoad"sv, Version::MAJOR, Version::MINOR, Version::PATCH);

    const auto ver = REL::Module::get().version();
    if (ver < SKSE::RUNTIME_SSE_1_6_1170)
    {
        logger::error("Unsupported runtime version {}"sv, ver);
        return;
    }

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(1 << 10);

    Settings::Load();

    if (Settings::General::bVerboseLogging.GetValue())
    {
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);

        logger::trace("enabled verbose logging"sv);
    }

    Patches::PreLoad();

    g_isPreloaded = true;
}

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

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    if (!g_isPreloaded)
    {
        // init with log
        SKSE::Init(a_skse);

        logger::error("plugin did not preload, please install the preloader");
        return false;
    }

	SKSE::Init(a_skse, false);

	logger::info("EngineFixes SKSE Load"sv, SKSE::GetPluginVersion());

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler))
    {
        logger::error("Failed to register messaging interface listener"sv);
        return false;
    }

    Fixes::Load();
    Patches::Load();

	return true;
}

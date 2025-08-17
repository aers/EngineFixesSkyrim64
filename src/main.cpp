#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

#include "settings.h"

#include "clean_cosaves.h"

#include "fixes/fixes.h"

#include "Patches/patches.h"
#include "Patches/save_added_sound_categories.h"

bool g_isPreloaded = false;

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
    switch (a_msg->type)
    {
    case SKSE::MessagingInterface::kDataLoaded:
        if (Settings::General::bCleanSKSECoSaves)
            Util::CoSaves::Clean();
        if (Settings::Patches::bSaveAddedSoundCategories)
            Patches::SaveAddedSoundCategories::LoadVolumes();
        break;
    case SKSE::MessagingInterface::kPostLoadGame:
        break;
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

#ifdef NDEBUG
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
	OpenLog();

    REX::INFO("EngineFixes PreLoad"sv);

    const auto ver = REL::Module::GetSingleton()->version();
    if (ver != SKSE::RUNTIME_1_6_1170)
    {
        REX::ERROR("Unsupported runtime version {}"sv, ver);
        return;
    }

    auto& trampoline = REL::GetTrampoline();
    trampoline.create(1 << 11);

    Settings::Load();

    if (Settings::General::bVerboseLogging)
    {
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);

        REX::TRACE("enabled verbose logging"sv);
    }

    Patches::PreLoad();

    g_isPreloaded = true;
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
    if (!g_isPreloaded)
    {
        // init with log
        SKSE::Init(a_skse);

        REX::ERROR("plugin did not preload, please install the preloader");
        return false;
    }

	SKSE::Init(a_skse, { .log = false});

	REX::INFO("EngineFixes v{} SKSE Load"sv, SKSE::GetPluginVersion());

    const auto messaging = SKSE::GetMessagingInterface();
    if (!messaging->RegisterListener("SKSE", MessageHandler))
    {
        REX::ERROR("Failed to register messaging interface listener"sv);
        return false;
    }

    Fixes::Load();
    Patches::Load();

	return true;
}

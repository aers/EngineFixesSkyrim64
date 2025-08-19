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
        if (Settings::General::bCleanSKSECoSaves.GetValue())
            Util::CoSaves::Clean();
        if (Settings::Patches::bSaveAddedSoundCategories.GetValue())
            Patches::SaveAddedSoundCategories::LoadVolumes();
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

    logger::info("EngineFixes PreLoad"sv);

    const auto ver = REL::Module::get().version();
    if (ver != SKSE::RUNTIME_SSE_1_6_1170)
    {
        logger::error("Unsupported runtime version {}"sv, ver);
        return;
    }

    auto& trampoline = SKSE::GetTrampoline();
    trampoline.create(1 << 11);

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

	logger::info("EngineFixes v{} SKSE Load"sv, SKSE::GetPluginVersion());

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

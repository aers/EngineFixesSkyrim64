#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/spdlog.h>

#include "settings.h"

#include "Patches/patches.h"

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

	REX::INFO("EngineFixes PreLoad"sv);

    Settings::Load();

	Patches::PreLoad();
}

extern "C" __declspec(dllexport) void __stdcall Initialize() {
	OpenLog();
}

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse, false);

	REX::INFO("EngineFixes v{} SKSE Load"sv, SKSE::GetPluginVersion());

    Patches::Load();

	return true;
}

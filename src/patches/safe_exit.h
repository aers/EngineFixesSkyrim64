#pragma once
#include "fixes/precomputed_paths.h"
#include "fixes/texture_load_crash.h"

#include <spdlog/spdlog.h>

namespace Patches::SafeExit
{
    namespace detail
    {
        inline void Shutdown()
        {
            if (Settings::Warnings::bTextureLoadFailed.GetValue() && TextureLoadCrash::detail::TotalLoadFails > 0) {
                logger::warn("a total of {} textures failed to load in this session"sv, TextureLoadCrash::detail::TotalLoadFails);
                std::wostringstream warningString;
                warningString << L"WARNING: " << TextureLoadCrash::detail::TotalLoadFails << L" textures failed to load during this session. Please check EngineFixes.log for more details."sv;
                REX::W32::MessageBoxW(nullptr, warningString.str().c_str(), L"Engine Fixes for Skyrim Special Edition", MB_OK);
            }
            if (Settings::Warnings::bPrecomputedPathHasErrors.GetValue() && PrecomputedPaths::detail::HasIssues) {
                std::wostringstream warningString;
                warningString << L"WARNING: a precomputed path had issues, please check your EngineFixes.log for more details."sv;
                REX::W32::MessageBoxW(nullptr, warningString.str().c_str(), L"Engine Fixes for Skyrim Special Edition", MB_OK);
            }
            spdlog::default_logger()->flush();
            ::TerminateProcess(::GetCurrentProcess(), EXIT_SUCCESS);
        }
    }

    inline void Install()
    {
        // Main::Shutdown called by WinMain
        REL::Relocation target{ RELOCATION_ID(35545, 36544), VAR_NUM(0x35, 0x1AE) };
        target.write_call<5>(detail::Shutdown);

        logger::info("installed safe exit patch"sv);
    }
}
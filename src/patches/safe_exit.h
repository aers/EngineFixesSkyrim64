#pragma once
#include <spdlog/spdlog.h>

namespace Patches::SafeExit
{
    namespace detail
    {
        inline void Shutdown()
        {
            spdlog::default_logger()->flush();
            ::TerminateProcess(::GetCurrentProcess(), EXIT_SUCCESS);
        }
    }

    inline void Install()
    {
        // Main::Shutdown called by WinMain
        REL::Relocation target { REL::ID(36544), 0x1AE };
        target.write_call<5>(detail::Shutdown);

        logger::info("installed safe exit patch"sv);
    }
}
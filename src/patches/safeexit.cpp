#include "offsets.h"

namespace
{
    void Shutdown()
    {
        spdlog::default_logger()->flush();
        ::TerminateProcess(::GetCurrentProcess(), EXIT_SUCCESS);
    }

    void Install()
    {
        REL::Relocation<std::uintptr_t> target{ offsets::SafeExit::WinMain };
        auto& trampoline = SKSE::GetTrampoline();
        trampoline.write_call<5>(target.address() + 0x1AE, Shutdown);  // Main::Shutdown
    }
}

namespace patches
{
    bool PatchSafeExit()
    {
        logger::trace("- safe exit patch -"sv);

        Install();

        logger::trace("success"sv);
        return true;
    }
}

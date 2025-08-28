#pragma once

namespace Fixes::NullProcessCrash
{
    namespace detail
    {
        inline RE::TESForm* GetEquippedLeftHand(RE::AIProcess* a_process)
        {
            return a_process ? a_process->GetEquippedLeftHand() : nullptr;
        }

        inline RE::TESForm* GetEquippedRightHand(RE::AIProcess* a_process)
        {
            return a_process ? a_process->GetEquippedRightHand() : nullptr;
        }
    }

    inline void Install()
    {
        auto& trampoline = SKSE::GetTrampoline();

        {
            const REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(0, 38899) };
            trampoline.write_call<5>(target.address() + 0x6C, detail::GetEquippedLeftHand);
            trampoline.write_call<5>(target.address() + 0x9C, detail::GetEquippedRightHand);
        }

        {
            const REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(0, 47338) };
            trampoline.write_call<5>(target.address() + 0x47, detail::GetEquippedLeftHand);
            trampoline.write_call<5>(target.address() + 0x56, detail::GetEquippedRightHand);
        }

        logger::info("installed null process crash fix");
    }
}
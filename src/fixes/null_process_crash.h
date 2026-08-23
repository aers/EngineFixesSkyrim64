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
        auto& trampoline = REL::GetTrampoline();

        {
            const REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37943, 38899) };
            trampoline.write_call5(target.address() + 0x6C, reinterpret_cast<std::uintptr_t>(detail::GetEquippedLeftHand));
            trampoline.write_call5(target.address() + 0x9C, reinterpret_cast<std::uintptr_t>(detail::GetEquippedRightHand));
        }

        {
            const REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(46074, 47338) };
            trampoline.write_call5(target.address() + 0x47, reinterpret_cast<std::uintptr_t>(detail::GetEquippedLeftHand));
            trampoline.write_call5(target.address() + 0x56, reinterpret_cast<std::uintptr_t>(detail::GetEquippedRightHand));
        }

        REX::INFO("installed null process crash fix");
    }
}
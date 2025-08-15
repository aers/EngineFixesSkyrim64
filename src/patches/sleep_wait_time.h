#pragma once

namespace Patches::SleepWaitTime
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            Patch(const std::uintptr_t a_address, const float a_val)
            {
                static float VAL = a_val;

                push(rax);

                mov(rax, REX::UNRESTRICTED_CAST<std::uintptr_t>(std::addressof(VAL)));
                comiss(xmm0, ptr[rax]);

                pop(rax);

                jmp(ptr[rip]);
                dq(a_address + 0x7);
            }
        };

        inline void Install()
        {
            REL::Relocation target{ REL::ID(52486), 0x1D0 };

            Patch p(target.address(), Settings::Patches::fSleepWaitTimeModifier.GetValue());
            p.ready();

            auto& trampoline = REL::GetTrampoline();
            trampoline.write_jmp<6>(target.address(), trampoline.allocate(p));
            REL::WriteSafeData<uint8_t>(target.address() + 0x6, REL::NOP);
        }
    }

    inline void Install()
    {
        detail::Install();

        REX::INFO("installed sleep wait timer patch"sv);
    }
}
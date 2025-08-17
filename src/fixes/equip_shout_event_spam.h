#pragma once

namespace Fixes::EquipShoutEventSpam
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(std::uintptr_t a_target)
            {
                Xbyak::Label exitLbl;
                Xbyak::Label exitIP;
                Xbyak::Label sendEvent;

                // rbp = Actor*
                // rdi = TESShout*

                cmp(ptr[rbp + 0x1E8], rdi);  // if (actor->equippedShout != shout)
                je(exitLbl);
                mov(ptr[rbp + 0x1E8], rdi);  // actor->equippedShout = shout;
                test(rdi, rdi);                       // if (shout)
                jz(exitLbl);
                jmp(ptr[rip + sendEvent]);

                L(exitLbl);
                jmp(ptr[rip + exitIP]);

                L(exitIP);
                dq(a_target + 0x8A); // SendEvent end

                L(sendEvent);
                dq(a_target + 0xC); // SendEvent begin
            }
        };
    }

    inline void Install()
    {
        REL::Relocation target { REL::ID(38770), 0x13D };

        detail::Patch p(target.address());
        p.ready();

        auto& trampoline = REL::GetTrampoline();
        target.write_jmp<5>(trampoline.allocate(p));

        REL::WriteSafeFill(target.address() + 5, REL::NOP, 7);

        REX::INFO("installed equip shout event spam fix"sv);
    }
}
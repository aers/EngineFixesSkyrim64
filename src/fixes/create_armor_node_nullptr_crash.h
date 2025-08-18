#pragma once

namespace Fixes::CreateArmorNodeNullPtrCrash
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(std::uintptr_t a_target)
            {
                Xbyak::Label patchedJmpLbl, contLbl, zeroLbl;

                // original instructions
                mov(esi, edi);
                test(r12, r12);
                jz(zeroLbl);
                jmp(ptr[rip + contLbl]);
                L(zeroLbl);
                jmp(ptr[rip + patchedJmpLbl]);

                L(contLbl);
                dq(a_target + 0x7);
                L(patchedJmpLbl);
                dq(a_target + 0x21F); // jump over code that will crash due to missing null check
            }
        };
    }

    inline void Install()
    {
        REL::Relocation target { REL::ID(15712), 0x51B };

        detail::Patch p(target.address());
        p.ready();

        auto& trampoline = SKSE::GetTrampoline();
        target.write_branch<5>(trampoline.allocate(p));

        logger::info("installed create armor node nullptr crash fix"sv);
    }
}
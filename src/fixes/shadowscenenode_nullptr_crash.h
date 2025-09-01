#pragma once

namespace Fixes::ShadowSceneNodeNullPtrCrash
{
    namespace detail
    {
        struct Patch : Xbyak::CodeGenerator
        {
            Patch(std::uintptr_t a_target)
            {
                Xbyak::Label contAddrLbl, zeroLbl, zeroAddrLbl;

                // check for NULL
                test(rax, rax);
                jz(zeroLbl);

                // original instructions
                call(ptr[rax + 0x18]);
                test(al, al);

                jz(zeroLbl);
                jmp(ptr[rip + contAddrLbl]);
                L(zeroLbl);
                jmp(ptr[rip + zeroAddrLbl]);

                L(contAddrLbl);
                dq(a_target + 0x5);
                L(zeroAddrLbl);
                dq(a_target + 0x1D);
            }
        };
    }

    inline void Install()
    {
        REL::Relocation target{ RELOCATION_ID(99708, 106342), 0x16 };

        detail::Patch p(target.address());
        p.ready();

        auto& trampoline = SKSE::GetTrampoline();
        target.write_branch<5>(trampoline.allocate(p));

        logger::info("installed shadow scene node nullptr crash fix"sv);
    }
}
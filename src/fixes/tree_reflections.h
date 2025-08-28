#pragma once

namespace Fixes::TreeReflections
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(std::uintptr_t a_target)
            {
                Xbyak::Label retnLabel;

                // current: if(bUseEarlyZ) v3 |= 0x10000u;
                // goal: if(bUseEarlyZ || v3 == 0) v3 |= 0x10000u;
                // if (bUseEarlyZ)
                // .text:0000000141318C50                 cmp     cs:bUseEarlyZ, r13b
                // need 6 bytes to branch jmp so enter here
                // enter 1428255
                // .text:0000000141428255                 jz      short loc_14142825B
                jnz("CONDITION_MET");
                // edi = v3
                // if (v3 == 0)
                test(edi, edi);
                jnz("JMP_OUT");
                // .text:0000000141318C59                 bts     edi, 10h
                L("CONDITION_MET");
                bts(edi, 0x10);
                L("JMP_OUT");
                // exit 1318C5D
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_target + 0x6);
            }
        };
    }

    inline void Install()
    {
        const auto handle = REX::W32::GetModuleHandleA("d3dcompiler_46e.dll");

        if (handle)
        {
            logger::trace("enb detected - disabling fix, please use ENB's tree reflection fix instead"sv);
            return;
        }

        REL::Relocation target { RELOCATION_ID(0, 107551), 0x35 };
        detail::Patch p(target.address());
        p.ready();

        auto& trampoline = SKSE::GetTrampoline();
        target.write_branch<6>(trampoline.allocate(p));

        logger::info("installed tree lod reflection fix");
    }
}
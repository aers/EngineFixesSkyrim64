#pragma once
namespace Fixes::BSLightingShaderParallaxBug
{
    namespace detail
    {
#ifdef SKYRIM_AE
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(const std::uintptr_t a_target)
            {
                // orig code
                test(eax, 0x21C00);
                mov(r9d, 1);
                cmovnz(ecx, r9d);

                // new code
                cmp(dword[rbp+0x1D0-0x210], 0x3);     // technique ID = PARALLAX
                cmovz(ecx, r9d);  // set eye update true

                // jmp out
                jmp(ptr[rip]);
                dq(a_target + 0xF);
            };
        };
#else
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(const std::uintptr_t a_target)
            {
                // orig code
                and_(eax, 0x21C00);
                cmovnz(edx, r8d);

                // new code
                cmp(ebx, 0x3);    // technique ID = PARALLAX
                cmovz(edx, r8d);  // set eye update true

                // jmp out
                jmp(ptr[rip]);
                dq(a_target + 0x9);
            };
        };
#endif
    }

    inline void Install()
    {
        REL::Relocation target { RELOCATION_ID(100565, 107300), VAR_NUM(0x577, 0xB5D) };

        detail::Patch p(target.address());
        auto& trampoline = SKSE::GetTrampoline();
        trampoline.write_branch<6>(target.address(), trampoline.allocate(p));

        logger::info("installed bslightingshader parallax bug fix"sv);
    }
}
#pragma once
namespace Fixes::BSLightingShaderParallaxBug
{
    namespace detail
    {
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
    }

    inline void Install()
    {
        REL::Relocation target { REL::ID(107300), 0xB5D };

        detail::Patch p(target.address());
        auto& trampoline = REL::GetTrampoline();
        trampoline.write_jmp<6>(target.address(), trampoline.allocate(p));

        REX::INFO("installed bslightingshader parallax bug fix");
    }
}
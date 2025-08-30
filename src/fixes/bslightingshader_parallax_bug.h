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
    }

    inline void Install()
    {
        REL::Relocation target { REL::ID(100565), 0x577 };

        detail::Patch p(target.address());
        auto& trampoline = SKSE::GetTrampoline();
        trampoline.write_branch<6>(target.address(), trampoline.allocate(p));

        logger::info("installed bslightingshader parallax bug fix"sv);
    }
}
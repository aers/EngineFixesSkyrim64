#pragma once

namespace Fixes::VerticalLookSensitivity
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(std::uintptr_t a_target)
            {
                Xbyak::Label retnLabel;
                Xbyak::Label magicLabel;

                movss(xmm4, dword[rip + magicLabel]);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_target + 0x8);

                L(magicLabel);
                dd(0x3CC0C0C0);  // 1 / 42.5
            }
        };

    }

    inline void Install()
    {
        constexpr std::array todo = {
            std::pair(49978, 0x71),
            std::pair(32370, 0x5F),
            std::pair(49839, 0x5F)
        };

        auto& trampoline = SKSE::GetTrampoline();

        for (auto& [id, offset] : todo)
        {
            REL::Relocation target {REL::ID(id), offset};

            detail::Patch p(target.address());
            p.ready();

            target.write_branch<6>(trampoline.allocate(p));

            REL::safe_fill(target.address() + 0x6, REL::NOP, 0x2);
        }

        logger::info("installed vertical look sensitivity fix"sv);
    }
}
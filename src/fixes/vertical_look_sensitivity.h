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

                mulss(xmm3, dword[rip + magicLabel]);
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
            std::pair(50914, 0x65),
            std::pair(33119, 0x53),
            std::pair(50770, 0x53)
        };

        auto& trampoline = REL::GetTrampoline();

        for (auto& [id, offset] : todo)
        {
            REL::Relocation target {REL::ID(id), offset};

            detail::Patch p(target.address());
            p.ready();

            target.write_jmp<6>(trampoline.allocate(p));

            REL::WriteSafeFill(target.address() + 0x6, REL::NOP, 0x2);
        }

        REX::INFO("installed vertical look sensitivity fix"sv);
    }
}
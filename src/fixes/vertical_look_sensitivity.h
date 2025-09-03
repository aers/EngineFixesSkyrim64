#pragma once

namespace Fixes::VerticalLookSensitivity
{
    namespace detail
    {
#ifdef SKYRIM_AE
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
#else
        struct Patch : Xbyak::CodeGenerator
        {
            Patch(std::uintptr_t a_hookTarget, std::uintptr_t a_frameTimer)
            {
                Xbyak::Label retnLabel;
                Xbyak::Label magicLabel;
                Xbyak::Label timerLabel;

                // enter 850D81
                // r8 is unused
                //.text:0000000140850D81                 movss   xmm4, cs:frame_timer_without_slow
                // use magic instead
                movss(xmm4, dword[rip+magicLabel]);
                //.text:0000000140850D89                 movaps  xmm3, xmm4
                // use timer
                mov(r8, ptr[rip + timerLabel]);
                movss(xmm3, dword[r8]);

                // exit 850D8C
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(a_hookTarget + 0xB);

                L(magicLabel);
                dd(0x3CC0C0C0);

                L(timerLabel);
                dq(a_frameTimer);
            }
        };
#endif

    }

    inline void Install()
    {
#ifdef SKYRIM_AE
        constexpr std::array todo = {
            std::pair(50914, 0x65),
            std::pair(33119, 0x53),
            std::pair(50770, 0x53)
        };
#else
        constexpr std::array todo = {
            std::pair(49978, 0x71),
            std::pair(32370, 0x5F),
            std::pair(49839, 0x5F)
        };
#endif

        auto& trampoline = SKSE::GetTrampoline();

        for (auto& [id, offset] : todo) {
            REL::Relocation target{ REL::ID(id), offset };
#ifdef SKYRIM_AE
            detail::Patch p(target.address());
#else
            REL::Relocation secondsSinceLastFrameRealTime{ REL::ID(523661) };
            detail::Patch p(target.address(), secondsSinceLastFrameRealTime.address());
#endif
            p.ready();

            target.write_branch<6>(trampoline.allocate(p));

            REL::safe_fill(target.address() + 0x6, REL::NOP, VAR_NUM(0x5, 0x2));
        }

        logger::info("installed vertical look sensitivity fix"sv);
    }
}
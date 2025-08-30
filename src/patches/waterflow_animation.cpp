#include "waterflow_animation.h"

namespace Patches::WaterflowAnimation::detail
{
    void PatchWaterflowAnimation()
    {
        // patch the main game loop to also update our timer
        {
            REL::Relocation mainLoopTarget{ REL::ID(35565), 0x252 };

            struct MainUpdateCode : Xbyak::CodeGenerator
            {
                explicit MainUpdateCode(const std::uintptr_t a_addr)
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label funcLabel;
                    Xbyak::Label applicationRunTimeLabel;

                    sub(rsp, 0x20);
                    call(ptr[rip + funcLabel]);
                    add(rsp, 0x20);

                    // orig code
                    mov(rdx, ptr[rip + applicationRunTimeLabel]);
                    mov(edx, dword[rdx]);

                    jmp(ptr[rip + retnLabel]);

                    L(funcLabel);
                    dq(SKSE::stl::unrestricted_cast<uintptr_t>(update_timer));

                    L(retnLabel);
                    dq(a_addr + 0x6);

                    L(applicationRunTimeLabel);
                    dq(g_ApplicationRunTime.address());
                }
            };

            MainUpdateCode p(mainLoopTarget.address());
            p.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(mainLoopTarget.address(), trampoline.allocate(p));
        }

        // patch the water shader to use our timer
        // 107363
        {
            REL::Relocation waterShaderSetupMaterialTarget{ REL::ID(100602), 0x4A9 };

            struct WaterShaderCode : Xbyak::CodeGenerator
            {
                explicit WaterShaderCode(const std::uintptr_t a_addr)
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label timerLabel;

                    // enter 130DFD9
                    // .text:000000014130DFD9                 movss   xmm1, cs:TIMER_DEFAULT
                    // .text:000000014130DFE1                 movss   dword ptr[rdx + rax * 4 + 0Ch], xmm1
                    mov(r9, ptr[rip + timerLabel]);  // r9 is safe to use, unused again until .text:000000014130E13C                 mov     r9, r12
                    movss(xmm1, dword[r9]);
                    movss(dword[rdx + rax * 4 + 0xC], xmm1);

                    // exit 130DFE7
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_addr + 0xE);

                    L(timerLabel);
                    dq(SKSE::stl::unrestricted_cast<std::uintptr_t>(&g_Timer));
                }
            };

            WaterShaderCode p(waterShaderSetupMaterialTarget.address());
            p.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(waterShaderSetupMaterialTarget.address(), trampoline.allocate(p));
        }
    }
}

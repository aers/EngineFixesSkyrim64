#include "waterflow_animation.h"

namespace Patches::WaterflowAnimation::detail
{
    void PatchWaterflowAnimation()
    {
        // patch the main game loop to also update our timer
        {
            REL::Relocation mainLoopTarget{ REL::ID(36564), 0x26B };

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
                    dq(REX::UNRESTRICTED_CAST<uintptr_t>(update_timer));

                    L(retnLabel);
                    dq(a_addr + 0x6);

                    L(applicationRunTimeLabel);
                    dq(g_ApplicationRunTime.address());
                }
            };

            MainUpdateCode p(mainLoopTarget.address());
            p.ready();

            auto& trampoline = REL::GetTrampoline();
            trampoline.write_jmp<6>(mainLoopTarget.address(), trampoline.allocate(p));
        }

        // patch the water shader to use our timer
        // 107363
        {
            REL::Relocation waterShaderSetupMaterialTarget{ REL::ID(107363), 0x4BC };

            struct WaterShaderCode : Xbyak::CodeGenerator
            {
                explicit WaterShaderCode(const std::uintptr_t a_addr)
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label timerLabel;

                    // enter 14E167C
                    // .text:000000014141CE5C                 movss   xmm0, cs:dword_141EA2E40
                    // .text:000000014141CE64                 movss   dword ptr [r10+rax*4+0Ch], xmm0
                    mov(r9, ptr[rip + timerLabel]);  // r9 is safe to use, unused again until .text:000000014130E13C                 mov     r9, r12
                    movss(xmm0, dword[r9]);
                    movss(dword[r10 + rax * 4 + 0xC], xmm0);

                    // exit 14E168B
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_addr + 0xE);

                    L(timerLabel);
                    dq(REX::UNRESTRICTED_CAST<std::uintptr_t>(&g_Timer));
                }
            };

            WaterShaderCode p(waterShaderSetupMaterialTarget.address());
            p.ready();

            auto& trampoline = REL::GetTrampoline();
            trampoline.write_jmp<6>(waterShaderSetupMaterialTarget.address(), trampoline.allocate(p));
        }
    }
}

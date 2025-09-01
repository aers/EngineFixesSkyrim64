#pragma once

namespace Fixes::BSLightingAmbientSpecular
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            Patch(std::uintptr_t a_ambientSpecularAndFresnel, std::uintptr_t a_addAmbientSpecularToSetupGeometry)
            {
                Xbyak::Label jmpOut;
                // hook: 0x130AB2D (in middle of SetupGeometry, right before if (rawTechnique & RAW_FLAG_SPECULAR), just picked a random place tbh
                // test
                test(dword[r13 + 0x94], 0x20000);  // RawTechnique & RAW_FLAG_AMBIENT_SPECULAR
                jz(jmpOut);
                // ambient specular
                push(rax);
                push(rdx);
                mov(rax, a_ambientSpecularAndFresnel);  // xmmword_1E3403C
                movups(xmm0, ptr[rax]);
#ifdef SKYRIM_AE
                mov(rax, qword[rsp + 0x2D0 - 0x260 + 0x10]);  // PixelShader
#else
                mov(rax, qword[rsp + 0x170 - 0x120 + 0x10]);
#endif
                movzx(edx, byte[rax + 0x46]);  // m_ConstantOffsets 0x6 (AmbientSpecularTintAndFresnelPower)
#ifdef SKYRIM_AE
                mov(rax, ptr[rdi + 8]);  // m_PerGeometry buffer (copied from SetupGeometry)
#else
                mov(rax, ptr[r15 + 8]);
#endif
                movups(ptr[rax + rdx * 4], xmm0);  // m_PerGeometry buffer offset 0x6
                pop(rdx);
                pop(rax);
                // original code
                L(jmpOut);
                test(dword[r13 + 0x94], 0x200);
                jmp(ptr[rip]);
                dq(a_addAmbientSpecularToSetupGeometry + 0xB);
            }
        };
    }

    inline void Install()
    {
        // remove invalid code from BSLightingShader::SetupMaterial
        REL::Relocation materialTarget{ RELOCATION_ID(100563, 107298), VAR_NUM(0x713, 0x8CF) };
        materialTarget.write_fill(REL::NOP, 0x20);

        // add new code to BSLightingShader::SetupGeometry
        const REL::Relocation geometryTarget{ RELOCATION_ID(100565, 107300), VAR_NUM(0xBAD, 0x1271) };
        const REL::Relocation constant{ RELOCATION_ID(513256, 390997) };

        detail::Patch p(constant.address(), geometryTarget.address());
        p.ready();

        auto& trampoline = SKSE::GetTrampoline();
        trampoline.write_branch<5>(geometryTarget.address(), trampoline.allocate(p));

        logger::info("installed BSLightingAmbientSpecular fix"sv);
    }
}
#pragma once

namespace Fixes::MemoryAccessErrors
{
    namespace detail
    {
        static void InstallSnowMaterialCaseFix()
        {
            struct Patch : Xbyak::CodeGenerator
            {
                Patch(std::uintptr_t a_vtbl, std::uintptr_t a_hook, std::uintptr_t a_exit)
                {
                    Xbyak::Label vtblAddr;
                    Xbyak::Label snowRetnLabel;
                    Xbyak::Label exitRetnLabel;

                    mov(rax, ptr[rip + vtblAddr]);
                    cmp(rax, qword[rbx]);
                    je("IS_SNOW");

                    // not snow, fill with 0 to disable effect
                    mov(eax, 0x00000000);
                    mov(dword[rcx + rdx * 4 + 0xC], eax);
                    mov(dword[rcx + rdx * 4 + 0x8], eax);
                    mov(dword[rcx + rdx * 4 + 0x4], eax);
                    mov(dword[rcx + rdx * 4], eax);
                    jmp(ptr[rip + exitRetnLabel]);

                    // is snow, jump out to original except our overwritten instruction
                    L("IS_SNOW");
                    movss(xmm2, dword[rbx + 0xAC]);
                    jmp(ptr[rip + snowRetnLabel]);

                    L(vtblAddr);
                    dq(a_vtbl);

                    L(snowRetnLabel);
                    dq(a_hook + 0x8);

                    L(exitRetnLabel);
                    dq(a_exit);
                }
            };

            REL::Relocation vtbl{ RE::BSLightingShaderMaterialSnow::VTABLE[0] };
            REL::Relocation funcHook{ RELOCATION_ID(100563, 107298), VAR_NUM(0x4E0, 0x6A6) };
            REL::Relocation funcExit{ RELOCATION_ID(100563, 107298), VAR_NUM(0x5B6, 0x770) };
            Patch           patch(vtbl.address(), funcHook.address(), funcExit.address());
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                funcHook.address(),
                trampoline.allocate(patch));
        }

        inline static REL::Relocation<decltype(&RE::BGSShaderParticleGeometryData::Load)> origLoad;

        // BGSShaderParticleGeometryData::Load
        inline bool Load(RE::BGSShaderParticleGeometryData* a_this, RE::TESFile* a_file)
        {
            const bool retVal = origLoad(a_this, a_file);

            // the game doesn't allow more than 10 here
            if (a_this->data.size() >= 12) {
                const auto particleDensity = a_this->data[11];
                if (particleDensity.f > 10.0)
                    a_this->data[11].f = 10.0f;
            }

            return retVal;
        }

        static void InstallShaderParticleGeometryDataLimit()
        {
            REL::Relocation vtbl{ RE::BGSShaderParticleGeometryData::VTABLE[0] };
            origLoad = vtbl.write_vfunc(0x6, Load);
        }

        inline void InstallBSShadowDirectionalLightUseAfterFree()
        {
            struct Patch : Xbyak::CodeGenerator
            {
                Patch()
                {
                    mov(r9, r15);
                    nop();
                    nop();
                    nop();
                    nop();
                }
            };

            Patch patch;
            patch.ready();

            REL::Relocation target{ RELOCATION_ID(101499, 108496), VAR_NUM(0x1AFD, 0x1BED) };
            target.write(std::span{ patch.getCode<const std::byte*>(), patch.getSize() });
        }
    }

    inline void Install()
    {
        detail::InstallSnowMaterialCaseFix();
        detail::InstallShaderParticleGeometryDataLimit();
        detail::InstallBSShadowDirectionalLightUseAfterFree();

        logger::info("installed misc memory access error fixes"sv);
    }
}
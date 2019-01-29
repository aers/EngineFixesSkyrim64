#include "RE/Actor.h"
#include "RE/BGSShaderParticleGeometryData.h"
#include "RE/TESFile.h"
#include "RE/TESObjectBOOK.h"  

#include <future>

#include "fixes.h"
#include "utils.h"


namespace fixes
{
    // TY SniffleMan https://github.com/SniffleMan/MiscFixesSSE
    typedef void _TESObjectBook_LoadBuffer(RE::TESObjectBOOK* a_this, RE::BGSLoadFormBuffer* a_buf);
    static _TESObjectBook_LoadBuffer * orig_LoadBuffer;

    RelocPtr<_TESObjectBook_LoadBuffer*> vtbl_LoadBuffer(TESObjectBook_vtbl_offset + (0x0F * 0x8));
    
    void hk_TESObjectBook_LoadBuffer(RE::TESObjectBOOK * thisPtr, RE::BGSLoadFormBuffer* a_buf)
    {
        using Flag = RE::TESObjectBOOK::Data::Flag;
        using Skill = RE::TESObjectBOOK::Data::Skill;

        orig_LoadBuffer(thisPtr, a_buf);

        if (thisPtr->data.teaches.skill == Skill::kNone) {
            if (thisPtr->TeachesSkill()) {
                thisPtr->data.flags &= ~Flag::kTeachesSkill;
            }
            if (thisPtr->TeachesSpell()) {
                thisPtr->data.flags &= ~Flag::kTeachesSpell;
            }
        }
    }

    bool PatchRemovedSpellBook()
    {
        _VMESSAGE("- Removed Spell Book -");
        orig_LoadBuffer = *vtbl_LoadBuffer;
        SafeWrite64(vtbl_LoadBuffer.GetUIntPtr(), GetFnAddr(hk_TESObjectBook_LoadBuffer));
        _VMESSAGE("success");
        return true;
    }

    class ActorEx : public RE::Actor
    {
    public:
        bool Hook_IsRunning()
        {
            return this ? IsRunning() : false;
        }
    };

    RelocAddr<uintptr_t> call_IsRunning(GameFunc_Native_IsRunning_offset + 0x22);

    bool PatchPerkFragmentIsRunning()
    {       
        _VMESSAGE("- ::IsRunning fix -");
        g_branchTrampoline.Write5Call(call_IsRunning.GetUIntPtr(), GetFnAddr(&ActorEx::Hook_IsRunning));
        _VMESSAGE("success");

        return true;
     }

    RelocAddr<uintptr_t> BSLightingShaderMaterialSnow_vtbl(BSLightingShaderMaterialSnow_vtbl_offset);
    RelocAddr<uintptr_t> BSLightingShader_SetupMaterial_Snow_Hook(BSLightingShader_SetupMaterial_Snow_Hook_offset);
    RelocAddr<uintptr_t> BSLightingShader_SetupMaterial_Snow_Exit(BSLightingShader_SetupMaterial_Snow_Exit_offset);

    typedef bool(*BGSShaderParticleGeometryData_LoadForm_)(RE::BGSShaderParticleGeometryData* thisPtr,
        RE::TESFile* modInfo);
    BGSShaderParticleGeometryData_LoadForm_ orig_BGSShaderParticleGeometryData_LoadForm;
    RelocPtr<BGSShaderParticleGeometryData_LoadForm_> vtbl_BGSShaderParticleGeometryData_LoadForm(vtbl_BGSShaderParticleGeometryData_LoadForm_offset); // vtbl[6]

    RelocAddr<uintptr_t> BadUse(BadUseFuncBase_offset + 0x1AFD);

    bool hk_BGSShaderParticleGeometryData_LoadForm(RE::BGSShaderParticleGeometryData* thisPtr, RE::TESFile* file)
    {
        const bool retVal = orig_BGSShaderParticleGeometryData_LoadForm(thisPtr, file);

        // the game doesn't allow more than 10 here
        if (thisPtr->data.count >= 12)
        {
            const auto particleDensity = thisPtr->data[11];
            if (particleDensity > 10.0)
                thisPtr->data[11] = 10.0f;
        }

        return retVal;
    }

    bool PatchMemoryAccessErrors()
    {
        _VMESSAGE("- memory access errors -");
        _VMESSAGE("patching BSLightingShader::SetupMaterial snow material case");
        {
            struct SetupMaterial_Snow_Hook_Code : Xbyak::CodeGenerator
            {
                SetupMaterial_Snow_Hook_Code(void* buf) : CodeGenerator(4096, buf)
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
                    dq(BSLightingShaderMaterialSnow_vtbl.GetUIntPtr());

                    L(snowRetnLabel);
                    dq(BSLightingShader_SetupMaterial_Snow_Hook.GetUIntPtr() + 0x8);

                    L(exitRetnLabel);
                    dq(BSLightingShader_SetupMaterial_Snow_Exit.GetUIntPtr());
                }
            };

            void* codeBuf = g_localTrampoline.StartAlloc();
            SetupMaterial_Snow_Hook_Code code(codeBuf);
            g_localTrampoline.EndAlloc(code.getCurr());

            g_branchTrampoline.Write6Branch(BSLightingShader_SetupMaterial_Snow_Hook.GetUIntPtr(),
                uintptr_t(code.getCode()));
        }
        _VMESSAGE("patching BGSShaderParticleGeometryData limit");

        orig_BGSShaderParticleGeometryData_LoadForm = *vtbl_BGSShaderParticleGeometryData_LoadForm;
        SafeWrite64(vtbl_BGSShaderParticleGeometryData_LoadForm.GetUIntPtr(), GetFnAddr(hk_BGSShaderParticleGeometryData_LoadForm));

        _VMESSAGE("patching BSShadowDirectionalLight use after free");
        {
            // Xbyak is used here to generate the ASM to use instead of just doing it by hand
            struct Patch : Xbyak::CodeGenerator
            {
                Patch(void* buf) : CodeGenerator(1024, buf)
                {
                    mov(r9, r15);
                    nop();
                    nop();
                    nop();
                    nop();
                }
            };

            void* patchBuf = g_localTrampoline.StartAlloc();
            Patch patch(patchBuf);
            g_localTrampoline.EndAlloc(patch.getCurr());

            for (UInt32 i = 0; i < patch.getSize(); ++i)
            {
                SafeWrite8(BadUse.GetUIntPtr() + i, *(patch.getCode() + i));
            }
        }
        _VMESSAGE("success");

        return true;
    }


    RelocAddr<uintptr_t> BSDistantTreeShader_VFunc3_Hook(bsdistanttreeshader_hook);

    bool PatchTreeReflections()
    {
        _VMESSAGE("- blocky tree reflections -");
        _VMESSAGE("patching BSDistantTreeShader vfunc 3");
        struct PatchTreeReflection_Code : Xbyak::CodeGenerator
        {
            PatchTreeReflection_Code(void* buf) : CodeGenerator(4096, buf)
            {
                Xbyak::Label retnLabel;

                // current: if(bUseEarlyZ) v3 |= 0x10000u;
                // goal: if(bUseEarlyZ || v3 == 0) v3 |= 0x10000u;
                // if (bUseEarlyZ)
                // .text:0000000141318C50                 cmp     cs:bUseEarlyZ, r13b
                // need 6 bytes to branch jmp so enter here
                // enter 1318C57
                // .text:0000000141318C57                 jz      short loc_141318C5D
                jnz("CONDITION_MET");
                // edi = v3
                // if (v3 == 0)
                test(edi, edi);
                jnz("JMP_OUT");
                // .text:0000000141318C59                 bts     edi, 10h
                L("CONDITION_MET");
                bts(edi, 0x10);
                L("JMP_OUT");
                // exit 1318C5D
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(BSDistantTreeShader_VFunc3_Hook.GetUIntPtr() + 0x6);
            }
        };

        void* codeBuf = g_localTrampoline.StartAlloc();
        PatchTreeReflection_Code code(codeBuf);
        g_localTrampoline.EndAlloc(code.getCurr());

        g_branchTrampoline.Write6Branch(BSDistantTreeShader_VFunc3_Hook.GetUIntPtr(), uintptr_t(code.getCode()));

        _VMESSAGE("success");
        return true;
    }

    RelocAddr<uintptr_t> CameraMove_Timer1(CameraMove_Timer1_offset); 
    RelocAddr<uintptr_t> CameraMove_Timer2(CameraMove_Timer2_offset);
    RelocAddr<uintptr_t> CameraMove_Timer3(CameraMove_Timer3_offset); 
    RelocAddr<uintptr_t> CameraMove_Timer4(CameraMove_Timer4_offset); 
    RelocAddr<uintptr_t> CameraMove_Timer5(CameraMove_Timer5_offset); 

    bool PatchSlowTimeCameraMovement()
    {
        _VMESSAGE("- slow time camera movement -");
        _VMESSAGE("patching camera movement to use frame timer that ignores slow time");
        // patch (+0x4)
        SafeWrite8(CameraMove_Timer1.GetUIntPtr(), 0x89);
        SafeWrite8(CameraMove_Timer2.GetUIntPtr(), 0x22);
        SafeWrite8(CameraMove_Timer3.GetUIntPtr(), 0xBB);
        SafeWrite8(CameraMove_Timer4.GetUIntPtr(), 0x1E);
        SafeWrite8(CameraMove_Timer5.GetUIntPtr(), 0x51);
        _VMESSAGE("success");

        return true;
    }

    RelocAddr<uintptr_t> MO5STypo(MO5STypo_offset);

    bool PatchMO5STypo()
    {
        _VMESSAGE("- MO5S Typo -");
        // Change "D" to "5"
        SafeWrite8(MO5STypo.GetUIntPtr(), 0x35);
        _VMESSAGE("success");

        return true;
    }

    errno_t hk_wcsrtombs_s(std::size_t* a_retval, char* a_dst, rsize_t a_dstsz, const wchar_t** a_src, rsize_t a_len, std::mbstate_t* a_ps)
    {
        int numChars = WideCharToMultiByte(CP_UTF8, 0, *a_src, a_len, NULL, 0, NULL, NULL);
        bool err;
        std::string str;
        if (a_src && numChars != 0 && numChars <= str.max_size()) {
            str.resize(numChars);
            char* dst = a_dst ? a_dst : str.data();
            err = WideCharToMultiByte(CP_UTF8, 0, *a_src, a_len, dst, numChars, NULL, NULL) ? false : true;
        }
        else {
            err = true;
        }

        if (err) {
            if (a_retval) {
                *a_retval = static_cast<std::size_t>(-1);
            }
            if (a_dst && a_dstsz != 0 && a_dstsz <= (std::numeric_limits<rsize_t>::max)()) {
                a_dst[0] = '\0';
            }
            return GetLastError();
        }

        if (a_retval) {
            *a_retval = static_cast<std::size_t>(numChars);
        }
        return 0;
    }


    bool PatchBethesdaNetCrash()
    {
        _VMESSAGE("- bethesda.net crash -");
        PatchIAT(GetFnAddr(hk_wcsrtombs_s), "API-MS-WIN-CRT-CONVERT-L1-1-0.dll", "wcsrtombs_s");
        _VMESSAGE("success");
        return true;
    }
}

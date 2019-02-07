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

        orig_LoadBuffer(thisPtr, a_buf);

        if (thisPtr->data.teaches.skill == RE::ActorValue::kNone) {
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
        if (thisPtr->data.GetSize() >= 12)
        {
            const auto particleDensity = thisPtr->data[11];
            if (particleDensity.f > 10.0)
                thisPtr->data[11].f = 10.0f;
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

        std::string str;
        char* dst = 0;
        rsize_t dstsz = 0;
        if (a_dst) {
            dst = a_dst;
            dstsz = a_dstsz;
        }
        else {
            str.resize(numChars);
            dst = str.data();
            dstsz = str.max_size();
        }

        bool err;
        if (a_src && numChars != 0 && numChars <= dstsz) {
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

    bool PatchEquipShoutEventSpam()
    {
        _VMESSAGE("- equip shout event spam - ");

        constexpr std::uintptr_t BRANCH_OFF = 0x17A;
        constexpr std::uintptr_t SEND_EVENT_BEGIN = 0x18A;
        constexpr std::uintptr_t SEND_EVENT_END = 0x236;
        constexpr std::size_t EQUIPPED_SHOUT = offsetof(RE::Actor, equippedShout);
        constexpr UInt32 BRANCH_SIZE = 5;
        constexpr UInt32 CODE_CAVE_SIZE = 16;
        constexpr UInt32 DIFF = CODE_CAVE_SIZE - BRANCH_SIZE;
        constexpr UInt8 NOP = 0x90;

        RelocAddr<std::uintptr_t> funcBase(Equip_Shout_Procedure_Function_offset);

        struct Patch : Xbyak::CodeGenerator
        {
            Patch(void* a_buf, UInt64 a_funcBase) : Xbyak::CodeGenerator(1024, a_buf)
            {
                Xbyak::Label exitLbl;
                Xbyak::Label exitIP;
                Xbyak::Label sendEvent;

                // r14 = Actor*
                // rdi = TESShout*

                cmp(ptr[r14 + EQUIPPED_SHOUT], rdi);	// if (actor->equippedShout != shout)
                je(exitLbl);
                mov(ptr[r14 + EQUIPPED_SHOUT], rdi);	// actor->equippedShout = shout;
                test(rdi, rdi);							// if (shout)
                jz(exitLbl);
                jmp(ptr[rip + sendEvent]);


                L(exitLbl);
                jmp(ptr[rip + exitIP]);

                L(exitIP);
                dq(a_funcBase + SEND_EVENT_END);

                L(sendEvent);
                dq(a_funcBase + SEND_EVENT_BEGIN);
            }
        };

        void* patchBuf = g_localTrampoline.StartAlloc();
        Patch patch(patchBuf, funcBase.GetUIntPtr());
        g_localTrampoline.EndAlloc(patch.getCurr());

        g_branchTrampoline.Write5Branch(funcBase.GetUIntPtr() + BRANCH_OFF, reinterpret_cast<std::uintptr_t>(patch.getCode()));

        for (UInt32 i = 0; i < DIFF; ++i) {
            SafeWrite8(funcBase.GetUIntPtr() + BRANCH_OFF + BRANCH_SIZE + i, NOP);
        }

        _VMESSAGE("installed patch for equip event spam (size == %zu)", patch.getSize());

        return true;
    }

    RelocAddr<uintptr_t> AddAmbientSpecularToSetupGeometry(0x0130AB2D);
    RelocAddr<uintptr_t> g_AmbientSpecularAndFresnel(0x01E3403C);

    bool PatchBSLightingAmbientSpecular()
    {
        _VMESSAGE("BSLightingAmbientSpecular fix");
        _VMESSAGE("nopping SetupMaterial case");
        constexpr byte nop = 0x90;

        RelocAddr<uintptr_t> DisableSetupMaterialAmbientSpecular(0x01309B03);
        constexpr uint8_t length = 0x20;

        for (int i = 0; i < length; ++i)
        {
            SafeWrite8(DisableSetupMaterialAmbientSpecular.GetUIntPtr() + i, nop);
        }            

        _VMESSAGE("Adding SetupGeometry case");

        struct Patch : Xbyak::CodeGenerator
        {
            Patch(void *a_buf) : Xbyak::CodeGenerator(1024, a_buf)
            {
                Xbyak::Label jmpOut;
                // hook: 0x130AB2D (in middle of SetupGeometry, right before if (rawTechnique & RAW_FLAG_SPECULAR), just picked a random place tbh
                // test
                test(dword[r13 + 0x94], 0x20000); // RawTechnique & RAW_FLAG_AMBIENT_SPECULAR
                jz(jmpOut);
                // ambient specular
                push(rcx);
                push(rax);
                push(rdx);
                mov(rax, qword[rsp + 0x170 - 0x120 + 0x18]); // PixelShader
                movzx(edx, byte[rax + 0x46]); // m_ConstantOffsets 0x6 (AmbientSpecularTintAndFresnelPower)
                mov(rcx, ptr[r15 + 8]);  // m_PerGeometry buffer (copied from SetupGeometry)
                mov(rax, g_AmbientSpecularAndFresnel.GetUIntPtr()); // xmmword_1E3403C
                movups(xmm0, ptr[rax]); 
                movups(ptr[rcx + rdx * 4], xmm0); // m_PerGeometry buffer offset 0x6
                pop(rdx);
                pop(rax);
                pop(rcx);

                // original code
                L(jmpOut);
                test(dword[r13 + 0x94], 0x200);
                jmp(ptr[rip]);
                dq(AddAmbientSpecularToSetupGeometry.GetUIntPtr() + 11);
            }
        };

        void* patchBuf = g_localTrampoline.StartAlloc();
        Patch patch(patchBuf);
        g_localTrampoline.EndAlloc(patch.getCurr());

        g_branchTrampoline.Write5Branch(AddAmbientSpecularToSetupGeometry.GetUIntPtr(), reinterpret_cast<std::uintptr_t>(patch.getCode()));

        _VMESSAGE("success");

        return true;
    }
}

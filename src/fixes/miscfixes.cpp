#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/Trampoline.h"

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
        using Flag = RE::OBJ_BOOK::Flag;

        orig_LoadBuffer(thisPtr, a_buf);

        if (thisPtr->data.teaches.actorValueToAdvance == RE::ActorValue::kNone) {
            if (thisPtr->TeachesSkill()) {
                thisPtr->data.flags &= ~Flag::kAdvancesActorValue;
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
        auto trampoline = SKSE::GetTrampoline();
        trampoline->Write5Call(call_IsRunning.GetUIntPtr(), GetFnAddr(&ActorEx::Hook_IsRunning));
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
        if (thisPtr->data.size() >= 12)
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
            struct SetupMaterial_Snow_Hook_Code : SKSE::CodeGenerator
            {
                SetupMaterial_Snow_Hook_Code() : SKSE::CodeGenerator()
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

            SetupMaterial_Snow_Hook_Code code;
            code.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(BSLightingShader_SetupMaterial_Snow_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
        }
        _VMESSAGE("patching BGSShaderParticleGeometryData limit");

        orig_BGSShaderParticleGeometryData_LoadForm = *vtbl_BGSShaderParticleGeometryData_LoadForm;
        SafeWrite64(vtbl_BGSShaderParticleGeometryData_LoadForm.GetUIntPtr(), GetFnAddr(hk_BGSShaderParticleGeometryData_LoadForm));

        _VMESSAGE("patching BSShadowDirectionalLight use after free");
        {
            // Xbyak is used here to generate the ASM to use instead of just doing it by hand
            struct Patch : SKSE::CodeGenerator
            {
                Patch() : SKSE::CodeGenerator(100)
                {
                    mov(r9, r15);
                    nop();
                    nop();
                    nop();
                    nop();
                }
            };

            Patch patch;
            patch.finalize();

            for (UInt32 i = 0; i < patch.getSize(); ++i)
            {
                SafeWrite8(BadUse.GetUIntPtr() + i, *(patch.getCode() + i));
            }
        }
        _VMESSAGE("success");

        return true;
    }


    RelocAddr<uintptr_t> BSDistantTreeShader_VFunc3_Hook(BSDistantTreeShader_hook_offset);

    bool PatchTreeReflections()
    {
        _VMESSAGE("- blocky tree reflections -");

		const auto handle = GetModuleHandleA("d3dcompiler_46e.dll");

		if (handle)
		{
			_VMESSAGE("enb detected - disabling fix, please use ENB's tree reflection fix instead");
			return true;
		}

        _VMESSAGE("patching BSDistantTreeShader vfunc 3");
        struct PatchTreeReflection_Code : SKSE::CodeGenerator
        {
            PatchTreeReflection_Code() : SKSE::CodeGenerator()
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

        PatchTreeReflection_Code code;
        code.finalize();

        auto trampoline = SKSE::GetTrampoline();
        trampoline->Write6Branch(BSDistantTreeShader_VFunc3_Hook.GetUIntPtr(), uintptr_t(code.getCode()));

        _VMESSAGE("success");
        return true;
    }

    RelocAddr<int16_t> CameraMove_Timer1(CameraMove_Timer1_offset); 
    RelocAddr<int16_t> CameraMove_Timer2(CameraMove_Timer2_offset);
    RelocAddr<int16_t> CameraMove_Timer3(CameraMove_Timer3_offset);
    RelocAddr<int16_t> CameraMove_Timer4(CameraMove_Timer4_offset);
    RelocAddr<int16_t> CameraMove_Timer5(CameraMove_Timer5_offset);

    bool PatchSlowTimeCameraMovement()
    {
        _VMESSAGE("- slow time camera movement -");
        _VMESSAGE("patching camera movement to use frame timer that ignores slow time");
        // patch (+0x4)
        SafeWrite16(CameraMove_Timer1.GetUIntPtr(), *(int16_t *)CameraMove_Timer1.GetUIntPtr() + 0x4);
        SafeWrite16(CameraMove_Timer2.GetUIntPtr(), *(int16_t *)CameraMove_Timer2.GetUIntPtr() + 0x4);
        SafeWrite16(CameraMove_Timer3.GetUIntPtr(), *(int16_t *)CameraMove_Timer3.GetUIntPtr() + 0x4);
        SafeWrite16(CameraMove_Timer4.GetUIntPtr(), *(int16_t *)CameraMove_Timer4.GetUIntPtr() + 0x4);
        SafeWrite16(CameraMove_Timer5.GetUIntPtr(), *(int16_t *)CameraMove_Timer5.GetUIntPtr() + 0x4);
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
        constexpr std::size_t EQUIPPED_SHOUT = offsetof(RE::Actor, selectedPower);
        constexpr UInt32 BRANCH_SIZE = 5;
        constexpr UInt32 CODE_CAVE_SIZE = 16;
        constexpr UInt32 DIFF = CODE_CAVE_SIZE - BRANCH_SIZE;
        constexpr UInt8 NOP = 0x90;

        RelocAddr<std::uintptr_t> funcBase(Equip_Shout_Procedure_Function_offset);

        struct Patch : SKSE::CodeGenerator
        {
            Patch(std::uintptr_t a_funcBase) : SKSE::CodeGenerator()
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

        Patch patch(funcBase.GetUIntPtr());
        patch.finalize();

        auto trampoline = SKSE::GetTrampoline();
        trampoline->Write5Branch(funcBase.GetUIntPtr() + BRANCH_OFF, reinterpret_cast<std::uintptr_t>(patch.getCode()));

        for (UInt32 i = 0; i < DIFF; ++i) {
            SafeWrite8(funcBase.GetUIntPtr() + BRANCH_OFF + BRANCH_SIZE + i, NOP);
        }

        _VMESSAGE("installed patch for equip event spam (size == %zu)", patch.getSize());

        return true;
    }

    RelocAddr<uintptr_t> AddAmbientSpecularToSetupGeometry(AddAmbientSpecularToSetupGeometry_offset);
    RelocAddr<uintptr_t> g_AmbientSpecularAndFresnel(g_AmbientSpecularAndFresnel_offset);
    RelocAddr<uintptr_t> DisableSetupMaterialAmbientSpecular(DisableSetupMaterialAmbientSpecular_offset);

    bool PatchBSLightingAmbientSpecular()
    {
        _VMESSAGE("BSLightingAmbientSpecular fix");
        _VMESSAGE("nopping SetupMaterial case");
        constexpr byte nop = 0x90;

        constexpr uint8_t length = 0x20;

        for (int i = 0; i < length; ++i)
        {
            SafeWrite8(DisableSetupMaterialAmbientSpecular.GetUIntPtr() + i, nop);
        }            

        _VMESSAGE("Adding SetupGeometry case");

        struct Patch : SKSE::CodeGenerator
        {
            Patch() : SKSE::CodeGenerator()
            {
                Xbyak::Label jmpOut;
                // hook: 0x130AB2D (in middle of SetupGeometry, right before if (rawTechnique & RAW_FLAG_SPECULAR), just picked a random place tbh
                // test
                test(dword[r13 + 0x94], 0x20000); // RawTechnique & RAW_FLAG_AMBIENT_SPECULAR
                jz(jmpOut);
                // ambient specular
                push(rax);
                push(rdx);
                mov(rax, g_AmbientSpecularAndFresnel.GetUIntPtr()); // xmmword_1E3403C
                movups(xmm0, ptr[rax]);
                mov(rax, qword[rsp + 0x170 - 0x120 + 0x10]); // PixelShader
                movzx(edx, byte[rax + 0x46]); // m_ConstantOffsets 0x6 (AmbientSpecularTintAndFresnelPower)
                mov(rax, ptr[r15 + 8]);  // m_PerGeometry buffer (copied from SetupGeometry)
                movups(ptr[rax + rdx * 4], xmm0); // m_PerGeometry buffer offset 0x6
                pop(rdx);
                pop(rax);
                // original code
                L(jmpOut);
                test(dword[r13 + 0x94], 0x200);
                jmp(ptr[rip]);
                dq(AddAmbientSpecularToSetupGeometry.GetUIntPtr() + 11);
            }
        };

        Patch patch;
        patch.finalize();

        auto trampoline = SKSE::GetTrampoline();
        trampoline->Write5Branch(AddAmbientSpecularToSetupGeometry.GetUIntPtr(), reinterpret_cast<std::uintptr_t>(patch.getCode()));

        _VMESSAGE("success");

        return true;
    }

    bool PatchGHeapLeakDetectionCrash()
    {
        _VMESSAGE("- GHeap leak detection crash fix -");

        constexpr std::uintptr_t START = 0x4B;
        constexpr std::uintptr_t END = 0x5C;
        constexpr UInt8 NOP = 0x90;
        RelocAddr<std::uintptr_t> funcBase(GHeap_Leak_Detection_Crash_offset);

        for (std::uintptr_t i = START; i < END; ++i) {
            SafeWrite8(funcBase.GetUIntPtr() + i, NOP);
        }
        
        _VMESSAGE("success");

        return true;
    }

    static int magic = 0x3CC0C0C0; // 1 / 42.5

    RelocPtr<float> FrameTimer_WithoutSlowTime(g_FrameTimer_NoSlowTime_offset);

    // ??_7ThirdPersonState@@6B@ vtbl last function + 0x71
    RelocAddr<uintptr_t> ThirdPersonState_Vfunc_Hook(ThirdPersonState_Vfunc_Hook_offset);
    // ??_7DragonCameraState@@6B@ vtbl last function + 0x5F
    RelocAddr<uintptr_t> DragonCameraState_Vfunc_Hook(DragonCameraState_Vfunc_Hook_offset);
    // ??_7HorseCameraState@@6B@ vtbl last function + 0x5F
    RelocAddr<uintptr_t> HorseCameraState_Vfunc_Hook(HorseCameraState_Vfunc_Hook_offset);

    bool PatchVerticalLookSensitivity()
    {
        _VMESSAGE("- Vertical Look Sensitivity -");

        _VMESSAGE("patching third person state...");
        {
            struct ThirdPersonStateHook_Code : SKSE::CodeGenerator
            {
                ThirdPersonStateHook_Code() : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label magicLabel;
                    Xbyak::Label timerLabel;

                    // enter 850D81
                    // r8 is unused
                    //.text:0000000140850D81                 movss   xmm4, cs:frame_timer_without_slow
                    // use magic instead
                    mov(r8, ptr[rip + magicLabel]);
                    movss(xmm4, dword[r8]);
                    //.text:0000000140850D89                 movaps  xmm3, xmm4
                    // use timer
                    mov(r8, ptr[rip + timerLabel]);
                    movss(xmm3, dword[r8]);

                    // exit 850D8C
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(ThirdPersonState_Vfunc_Hook.GetUIntPtr() + 0xB);

                    L(magicLabel);
                    dq(uintptr_t(&magic));

                    L(timerLabel);
                    dq(FrameTimer_WithoutSlowTime.GetUIntPtr());
                }
            };

            ThirdPersonStateHook_Code code;
            code.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(ThirdPersonState_Vfunc_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
        }
        _VMESSAGE("success");

        _VMESSAGE("patching dragon camera state...");
        {
            struct DragonCameraStateHook_Code : SKSE::CodeGenerator
            {
                DragonCameraStateHook_Code() : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label magicLabel;
                    Xbyak::Label timerLabel;

                    // enter 850D81
                    // r8 is unused
                    //.text:0000000140850D81                 movss   xmm4, cs:frame_timer_without_slow
                    // use magic instead
                    mov(r8, ptr[rip + magicLabel]);
                    movss(xmm4, dword[r8]);
                    //.text:0000000140850D89                 movaps  xmm3, xmm4
                    // use timer
                    mov(r8, ptr[rip + timerLabel]);
                    movss(xmm3, dword[r8]);

                    // exit 850D8C
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(DragonCameraState_Vfunc_Hook.GetUIntPtr() + 0xB);

                    L(magicLabel);
                    dq(uintptr_t(&magic));

                    L(timerLabel);
                    dq(FrameTimer_WithoutSlowTime.GetUIntPtr());
                }
            };

            DragonCameraStateHook_Code code;
            code.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(DragonCameraState_Vfunc_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
        }
        _VMESSAGE("success");

        _VMESSAGE("patching horse camera state...");
        {
            struct HorseCameraStateHook_Code : SKSE::CodeGenerator
            {
                HorseCameraStateHook_Code() : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label magicLabel;
                    Xbyak::Label timerLabel;

                    // enter 850D81
                    // r8 is unused
                    //.text:0000000140850D81                 movss   xmm4, cs:frame_timer_without_slow
                    // use magic instead
                    mov(r8, ptr[rip + magicLabel]);
                    movss(xmm4, dword[r8]);
                    //.text:0000000140850D89                 movaps  xmm3, xmm4
                    // use timer
                    mov(r8, ptr[rip + timerLabel]);
                    movss(xmm3, dword[r8]);

                    // exit 850D8C
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(HorseCameraState_Vfunc_Hook.GetUIntPtr() + 0xB);

                    L(magicLabel);
                    dq(uintptr_t(&magic));

                    L(timerLabel);
                    dq(FrameTimer_WithoutSlowTime.GetUIntPtr());
                }
            };

            HorseCameraStateHook_Code code;
            code.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(HorseCameraState_Vfunc_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
        }
        _VMESSAGE("success");

        return true;
    }

	class EnchantmentItemEx : public RE::EnchantmentItem
	{
	public:
		using func_t = decltype(&RE::EnchantmentItem::GetNoAbsorb); // 5E
		static inline REL::Function<func_t> func;


		bool Hook_DisallowsAbsorbReflection()
		{
			using Archetype = RE::EffectArchetypes::ArchetypeID;
			for (auto& effect : effects) {
				if (effect->baseEffect->HasArchetype(Archetype::kSummonCreature)) {
					return true;
				}
			}
			return func(this);
		}


		static void InstallHooks()
		{
			// ??_7EnchantmentItem@@6B@
			REL::Offset<std::uintptr_t> vTbl(offset_vtbl_EnchantmentItem + (0x8 * 0x5E));
            func = vTbl.WriteVFunc(0x5E, &EnchantmentItemEx::Hook_DisallowsAbsorbReflection);
			_DMESSAGE("Installed hook for (%s)", typeid(EnchantmentItemEx).name());
		}
	};

	bool PatchConjurationEnchantAbsorbs()
	{
		_VMESSAGE("- Enchantment Absorption on Staff Summons -");
		EnchantmentItemEx::InstallHooks();
		_VMESSAGE("success");

		return true;
	}

    class ArcheryDownwardAiming
    {
    public:
        static void Install()
        {
            REL::Offset<std::uintptr_t> funcBase(CalculateCollisionCall_offset);
            auto trampoline = SKSE::GetTrampoline();
            _Move = trampoline->Write5CallEx(funcBase.GetAddress() + 0x3E9, &ArcheryDownwardAiming::Hook_Move);
        }

    private:
        static void Hook_Move(RE::Projectile* a_this, /*const*/ RE::NiPoint3& a_from, const RE::NiPoint3& a_to)
        {
            auto refShooter = a_this->shooter.get();
            if (refShooter && refShooter->Is(RE::FormType::ActorCharacter)) {
                auto akShooter = static_cast<RE::Actor*>(refShooter.get());
                [[maybe_unused]] RE::NiPoint3 direction;
                akShooter->GetEyeVector(a_from, direction, true);
            }

            _Move(a_this, a_from, a_to);
        }


        using Move_t = decltype(&ArcheryDownwardAiming::Hook_Move);
        static inline REL::Function<Move_t> _Move;
    };

	bool PatchArcheryDownwardAiming()
	{
		_VMESSAGE("- archery downward aiming -");
        ArcheryDownwardAiming::Install();
		_VMESSAGE("- success -");

		return true;
	}

  RelocAddr<uintptr_t> AnimationSignedCrash(offset_AnimationLoadSigned);

  bool PatchAnimationLoadSignedCrash()
  {
      _VMESSAGE("- animation load crash -");
      // Change "BF" to "B7"
      SafeWrite8(AnimationSignedCrash.GetUIntPtr(), 0xB7);
      _VMESSAGE("success");

    return true;
  }

	bool PatchLipSync()
	{			   
		_VMESSAGE("- lip sync bug -");
		constexpr UInt8 OFFSETS[] = {
			0x1E,
			0x3A,
			0x9A,
			0xD8
		};

		constexpr std::size_t NUM_OFFSETS = std::extent<decltype(OFFSETS)>::value;

		REL::Offset<std::uintptr_t> funcBase(LipSync_FUNC_ADDR);
		for (std::size_t i = 0; i < NUM_OFFSETS; ++i) {
			SafeWrite8(funcBase.GetAddress() + OFFSETS[i], 0xEB);    // jns -> jmp
		}
		_VMESSAGE("- success -");

		return true;
	}

	class CalendarEx
	{
	public:
		static void AdvanceTime(float a_secondsPassed)
		{
			auto time = RE::Calendar::GetSingleton();
			float hoursPassed = (a_secondsPassed * time->timeScale->value / (60.0 * 60.0)) + time->gameHour->value - 24.0;
			if (hoursPassed > 24.0) {
				do {
					time->midnightsPassed += 1;
					time->rawDaysPassed += 1.0;
					hoursPassed -= 24.0;
				} while (hoursPassed > 24.0);
				time->gameDaysPassed->value = (hoursPassed / 24.0) + time->rawDaysPassed;
			}
		}


		static void InstallHooks()
		{
			constexpr std::size_t CAVE_START = 0x17A;
			constexpr std::size_t CAVE_SIZE = 0x15;

			REL::Offset<std::uintptr_t> funcBase(Calendar_AdvanceTime_call_offset);

			struct Patch : SKSE::CodeGenerator
			{
				Patch(std::uintptr_t a_addr) : SKSE::CodeGenerator(CAVE_SIZE)
				{
					Xbyak::Label jmpLbl;

					movaps(xmm0, xmm1);
					jmp(ptr[rip + jmpLbl]);

					L(jmpLbl);
					dq(a_addr);
				}
			};

			Patch patch(unrestricted_cast<std::uintptr_t>(&CalendarEx::AdvanceTime));
            patch.finalize();

			for (std::size_t i = 0; i < patch.getSize(); ++i) {
				SafeWrite8(funcBase.GetAddress() + CAVE_START + i, patch.getCode()[i]);
			}
		}
	};

	bool PatchCalendarSkipping()
	{
		_VMESSAGE("-calendar skipping-");
		CalendarEx::InstallHooks();
		_VMESSAGE("success");

		return true;
	}

    class GetKeywordItemCount
    {
    public:
        static bool Execute(const RE::SCRIPT_PARAMETER* a_paramInfo, RE::SCRIPT_FUNCTION::ScriptData* a_scriptData, RE::TESObjectREFR* a_thisObj, RE::TESObjectREFR* a_containingObj, RE::Script* a_scriptObj, RE::ScriptLocals* a_locals, double& a_result, UInt32& a_opcodeOffsetPtr)
        {
            if (!a_scriptObj || a_scriptObj->refObjects.empty()) {
                return false;
            }

            auto param = a_scriptObj->refObjects.front();
            if (!param->form || param->form->IsNot(RE::FormType::Keyword)) {
                return false;
            }

            return Eval(a_thisObj, param->form, 0, a_result);
        }


        static bool Eval(RE::TESObjectREFR* a_thisObj, void* a_param1, void* a_param2, double& a_result)
        {
            a_result = 0.0;
            if (!a_thisObj || !a_param1) {
                return true;
            }

            auto log = RE::ConsoleLog::GetSingleton();
            if (!a_thisObj->GetContainer()) {
                if (log->IsConsoleMode()) {
                    log->Print("Calling Reference is not a Container Object");
                }
                return true;
            }

            auto keyword = static_cast<RE::BGSKeyword*>(a_param1);
            auto inv = a_thisObj->GetInventoryCounts([&](RE::TESBoundObject* a_object) -> bool
            {
                auto keywordForm = a_object->As<RE::BGSKeywordForm>();
                return keywordForm && keywordForm->HasKeyword(keyword);
            });

            for (auto& elem : inv) {
                a_result += elem.second;
            }

            if (log->IsConsoleMode()) {
                log->Print("GetKeywordItemCount >> %0.2f", a_result);
            }

            return true;
        }


        static void Register()
        {
            auto command = RE::SCRIPT_FUNCTION::LocateScriptCommand(LONG_NAME);
            if (command) {
                command->executeFunction = Execute;
                command->conditionFunction = Eval;
            }
        }


        static constexpr char LONG_NAME[] = "GetKeywordItemCount";
    };

	bool PatchGetKeywordItemCount()
	{
        _VMESSAGE("-broken GetKeywordItemCount condition function-");
        GetKeywordItemCount::Register();
        _VMESSAGE("success");

        return true;
	}
}

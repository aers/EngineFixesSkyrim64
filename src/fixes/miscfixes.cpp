#include "fixes.h"

namespace fixes
{
    class AnimationLoadSignedCrashPatch
    {
    public:
        static void Install()
        {
            // Change "BF" to "B7"
            REL::Relocation<std::uintptr_t> target{ REL::ID(64198), 0x91 };
            REL::safe_write(target.address(), std::uint8_t{ 0xB7 });
        }
    };

    bool PatchAnimationLoadSignedCrash()
    {
        logger::trace("- animation load signed crash fix -"sv);

        AnimationLoadSignedCrashPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class ArcheryDownwardAimingPatch
    {
    public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> funcBase{ REL::ID(42852) };
            auto& trampoline = SKSE::GetTrampoline();
            _Move = trampoline.write_call<5>(funcBase.address() + 0x3E9, Move);
        }

    private:
        static void Move(RE::Projectile* a_this, /*const*/ RE::NiPoint3& a_from, const RE::NiPoint3& a_to)
        {
            auto refShooter = a_this->shooter.get();
            if (refShooter && refShooter->Is(RE::FormType::ActorCharacter))
            {
                auto akShooter = static_cast<RE::Actor*>(refShooter.get());
                [[maybe_unused]] RE::NiPoint3 direction;
                akShooter->GetEyeVector(a_from, direction, true);
            }

            _Move(a_this, a_from, a_to);
        }

        static inline REL::Relocation<decltype(Move)> _Move;
    };

    bool PatchArcheryDownwardAiming()
    {
        logger::trace("- archery downward aiming fix -"sv);

        ArcheryDownwardAimingPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class BethesdaNetCrashPatch
    {
    public:
        static void Install()
        {
            SKSE::PatchIAT(hk_wcsrtombs_s, "API-MS-WIN-CRT-CONVERT-L1-1-0.dll", "wcsrtombs_s");
        }

    private:
        static errno_t hk_wcsrtombs_s(std::size_t* a_retval, char* a_dst, rsize_t a_dstsz, const wchar_t** a_src, rsize_t a_len, [[maybe_unused]] std::mbstate_t* a_ps)
        {
            const auto numChars = WideCharToMultiByte(CP_UTF8, 0, *a_src, static_cast<int>(a_len), nullptr, 0, nullptr, nullptr);

            std::string str;
            char* dst = nullptr;
            rsize_t dstsz = 0;
            if (a_dst)
            {
                dst = a_dst;
                dstsz = a_dstsz;
            }
            else
            {
                str.resize(numChars);
                dst = str.data();
                dstsz = str.max_size();
            }

            bool err;
            if (a_src && numChars != 0 && numChars <= dstsz)
                err = WideCharToMultiByte(CP_UTF8, 0, *a_src, static_cast<int>(a_len), dst, numChars, nullptr, nullptr) ? false : true;
            else
                err = true;

            if (err)
            {
                if (a_retval)
                    *a_retval = static_cast<std::size_t>(-1);
                if (a_dst && a_dstsz != 0 && a_dstsz <= (std::numeric_limits<rsize_t>::max)())
                    a_dst[0] = '\0';
                return GetLastError();
            }

            if (a_retval)
                *a_retval = static_cast<std::size_t>(numChars);

            return 0;
        }
    };

    bool PatchBethesdaNetCrash()
    {
        logger::trace("- bethesda.net crash fix -"sv);

        BethesdaNetCrashPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class BSLightingAmbientSpecularPatch
    {
    public:
        static void Install()
        {
            logger::trace("nopping SetupMaterial case"sv);

            constexpr std::uint8_t nop = 0x90;
            constexpr std::size_t length = 0x20;

            REL::Relocation<std::uintptr_t> addAmbientSpecularToSetupGeometry{ REL::ID(100565), 0xBAD };
            REL::Relocation<std::uintptr_t> ambientSpecularAndFresnel{ REL::ID(513256) };
            REL::Relocation<std::uintptr_t> disableSetupMaterialAmbientSpecular{ REL::ID(100563), 0x713 };

            for (std::size_t i = 0; i < length; ++i)
                REL::safe_write(disableSetupMaterialAmbientSpecular.address() + i, nop);

            logger::trace("Adding SetupGeometry case"sv);

            struct Patch : Xbyak::CodeGenerator
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
                    mov(rax, qword[rsp + 0x170 - 0x120 + 0x10]);  // PixelShader
                    movzx(edx, byte[rax + 0x46]);                 // m_ConstantOffsets 0x6 (AmbientSpecularTintAndFresnelPower)
                    mov(rax, ptr[r15 + 8]);                       // m_PerGeometry buffer (copied from SetupGeometry)
                    movups(ptr[rax + rdx * 4], xmm0);             // m_PerGeometry buffer offset 0x6
                    pop(rdx);
                    pop(rax);
                    // original code
                    L(jmpOut);
                    test(dword[r13 + 0x94], 0x200);
                    jmp(ptr[rip]);
                    dq(a_addAmbientSpecularToSetupGeometry + 11);
                }
            };

            Patch patch(ambientSpecularAndFresnel.address(), addAmbientSpecularToSetupGeometry.address());
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<5>(
                addAmbientSpecularToSetupGeometry.address(),
                trampoline.allocate(patch));
        }
    };

    bool PatchBSLightingAmbientSpecular()
    {
        logger::trace("- BSLightingAmbientSpecular fix -"sv);

        BSLightingAmbientSpecularPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    bool PatchBSTempEffectNiRTTI()
    {
        logger::trace("- BSTempEffect NiRTTI fix -");

        REL::Relocation<RE::NiRTTI*> rttiBSTempEffect{ RE::BSTempEffect::Ni_RTTI };
        REL::Relocation<RE::NiRTTI*> rttiNiObject{ RE::NiObject::Ni_RTTI };
        rttiBSTempEffect->baseRTTI = rttiNiObject.get();

        logger::trace("success");
        return true;
    }

    class CalendarSkippingPatch
    {
    public:
        static void Install()
        {
            std::array targets{
                std::make_pair(13183, 0xE2),
                std::make_pair(35565, 0x24D),
                std::make_pair(35567, 0x3A),
                std::make_pair(39373, 0x2B1),
                std::make_pair(39410, 0x78),
            };

            auto& trampoline = SKSE::GetTrampoline();
            for (const auto& [id, offset] : targets)
            {
                REL::Relocation<std::uintptr_t> target{ REL::ID(id), offset };
                _Update = trampoline.write_call<5>(target.address(), Update);
            }
        }

    private:
        static void Update(RE::Calendar* a_this, float a_seconds)
        {
            _Update(a_this, a_seconds);

            float hoursPassed = (a_seconds * a_this->timeScale->value / (60.0F * 60.0F)) + a_this->gameHour->value - 24.0F;
            if (hoursPassed > 24.0)
            {
                do
                {
                    a_this->midnightsPassed += 1;
                    a_this->rawDaysPassed += 1.0F;
                    hoursPassed -= 24.0F;
                } while (hoursPassed > 24.0F);
                a_this->gameDaysPassed->value = (hoursPassed / 24.0F) + a_this->rawDaysPassed;
            }
        }

        static inline REL::Relocation<decltype(Update)> _Update;
    };

    bool PatchCalendarSkipping()
    {
        logger::trace("- calendar skipping fix -"sv);

        CalendarSkippingPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class CellInitPatch
    {
    public:
        static void Install()
        {
            auto& trampoline = SKSE::GetTrampoline();
            REL::Relocation<std::uintptr_t> funcBase{ REL::ID(18474) };
            _GetLocation = trampoline.write_call<5>(funcBase.address() + 0x110, GetLocation);
        }

    private:
        static RE::BGSLocation* GetLocation(const RE::ExtraDataList* a_this)
        {
            auto cell = adjust_pointer<RE::TESObjectCELL>(a_this, -0x48);
            auto loc = _GetLocation(a_this);
            if (!cell->IsInitialized())
            {
                auto file = cell->GetFile();
                auto formID = static_cast<RE::FormID>(reinterpret_cast<std::uintptr_t>(loc));
                RE::TESForm::AddCompileIndex(formID, file);
                loc = RE::TESForm::LookupByID<RE::BGSLocation>(formID);
            }
            return loc;
        }

        static inline REL::Relocation<decltype(GetLocation)> _GetLocation;
    };

    bool PatchCellInit()
    {
        logger::trace("- cell init fix -"sv);

        CellInitPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    // TODO: cleanup
    // Patch for 1.5.97 but should be version-independent
    bool PatchCreateArmorNodeNullptrCrash()
    {
        logger::trace("- CreateArmorNode subfunction crash fix -"sv);

        // sub_1401CAFB0
        const std::uint64_t faddr = REL::ID(15535).address();

        /*
        loc_1401CB520:
          [...]
        .text:00000001401CB538                 mov     esi, ebx
        .text:00000001401CB53A                 test    r12, r12
        .text:00000001401CB53D                 jz      short loc_1401CB5B6 // <-- this jump needs to be patched but the operand is too small to fit
        */
        static const std::uint8_t expected1[] = {
            0x8B, 0xF3,        // mov     esi, ebx
            0x4D, 0x85, 0xE4,  // test    r12, r12
            0x74               // jz short (opcode only); +1 byte displacement
        };

        const std::uint8_t* loc1p = (std::uint8_t*)(std::uintptr_t)(faddr + 0x588);
        if (memcmp(loc1p, expected1, sizeof(expected1)))
        {
            logger::trace("Code at first offset is not as expected, skipping patch"sv);
            return false;
        }

        const std::int8_t disp8 = loc1p[sizeof(expected1)];  // jz short displacement (should be 0x77)
        const std::uint8_t* const loc1end = loc1p + sizeof(expected1) + 1;

        /*
        loc_1401CB5B6:
        .text:00000001401CB5B6                 mov     rdi, [r12+130h] // <-- this is where the crash happens, because r12 == 0
        .text:00000001401CB5BE                 test    rdi, rdi
        .text:00000001401CB5C1                 jz      loc_1401CB751 // <-- This is the location we want to jump to when test r12,r12 fails
        */

        const void* const loc2p = loc1end + disp8;
        const std::ptrdiff_t offs2 = (std::uintptr_t)loc2p - faddr;
        logger::trace(FMT_STRING("Located second block at offset 0x{:X}"), offs2);

        static const std::uint8_t expected2[] =  // this should use scanning instead; hardcoding offset 130h is bad
            {
                0x49, 0x8B, 0xBC, 0x24, 0x30, 0x01, 0x00, 0x00,  // mov     rdi, [r12+130h]
                0x48, 0x85, 0xFF,                                // test    rdi, rdi
                0x0F, 0x84                                       // jz (opcode only); +4 bytes displacement
            };

        if (std::memcmp(loc2p, expected2, sizeof(expected2)))
        {
            logger::trace("Code at second offset is not as expected, skipping patch"sv);
            return false;
        }

        logger::trace("Code confirmed broken; installing fix"sv);

        const std::uint8_t* const loc2end = (const std::uint8_t*)loc2p + sizeof(expected2) + sizeof(std::int32_t);

        std::int32_t disp2 = *unrestricted_cast<std::int32_t*>(((const std::uint8_t*)loc2p) + sizeof(expected2));
        const std::uint8_t* const target = loc2end + disp2;

        struct Code : Xbyak::CodeGenerator
        {
            Code(std::uintptr_t a_contAddr, std::uintptr_t a_patchedAddr)
            {
                Xbyak::Label patchedJmpLbl, contLbl, zeroLbl;

                // original instructions minus jz short
                db(expected1, sizeof(expected1) - 1);

                jz(zeroLbl);              // patched non-short jz
                jmp(ptr[rip + contLbl]);  // continue original code
                L(zeroLbl);               // was zero; skip over code that would crash
                jmp(ptr[rip + patchedJmpLbl]);

                L(contLbl);
                dq(a_contAddr);
                L(patchedJmpLbl);
                dq(a_patchedAddr);
            }
        };

        Code code(unrestricted_cast<std::uintptr_t>(loc1end), unrestricted_cast<std::uintptr_t>(target));
        code.ready();

        auto& trampoline = SKSE::GetTrampoline();
        if (!trampoline.write_branch<5>(unrestricted_cast<std::uintptr_t>(loc1p), trampoline.allocate(code)))
            return false;

        logger::trace("success"sv);
        return true;
    }

    class ConjurationEnchantAbsorbsPatch
    {
    public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> vtbl{ REL::ID(228570) };
            _DisallowsAbsorbReflection = vtbl.write_vfunc(0x5E, DisallowsAbsorbReflection);
        }

    private:
        static bool DisallowsAbsorbReflection(RE::EnchantmentItem* a_this)
        {
            using Archetype = RE::EffectArchetypes::ArchetypeID;
            for (const auto& effect : a_this->effects)
            {
                if (effect->baseEffect->HasArchetype(Archetype::kSummonCreature))
                {
                    return true;
                }
            }
            return _DisallowsAbsorbReflection(a_this);
        }

        using DisallowsAbsorbReflection_t = decltype(&RE::EnchantmentItem::GetNoAbsorb);  // 5E
        static inline REL::Relocation<DisallowsAbsorbReflection_t> _DisallowsAbsorbReflection;
    };

    bool PatchConjurationEnchantAbsorbs()
    {
        logger::trace("- enchantment absorption on staff summons fix -"sv);

        ConjurationEnchantAbsorbsPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class EquipShoutEventSpamPatch
    {
    public:
        static void Install()
        {
            constexpr std::uintptr_t BRANCH_OFF = 0x17A;
            constexpr std::uintptr_t SEND_EVENT_BEGIN = 0x18A;
            constexpr std::uintptr_t SEND_EVENT_END = 0x236;
            constexpr std::size_t EQUIPPED_SHOUT = offsetof(RE::Actor, selectedPower);
            constexpr std::uint32_t BRANCH_SIZE = 5;
            constexpr std::uint32_t CODE_CAVE_SIZE = 16;
            constexpr std::uint32_t DIFF = CODE_CAVE_SIZE - BRANCH_SIZE;
            constexpr std::uint8_t NOP = 0x90;

            REL::Relocation<std::uintptr_t> funcBase{ REL::ID(37821) };

            struct Patch : Xbyak::CodeGenerator
            {
                Patch(std::uintptr_t a_funcBase)
                {
                    Xbyak::Label exitLbl;
                    Xbyak::Label exitIP;
                    Xbyak::Label sendEvent;

                    // r14 = Actor*
                    // rdi = TESShout*

                    cmp(ptr[r14 + EQUIPPED_SHOUT], rdi);  // if (actor->equippedShout != shout)
                    je(exitLbl);
                    mov(ptr[r14 + EQUIPPED_SHOUT], rdi);  // actor->equippedShout = shout;
                    test(rdi, rdi);                       // if (shout)
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

            Patch patch(funcBase.address());
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<5>(
                funcBase.address() + BRANCH_OFF,
                trampoline.allocate(patch));

            for (std::uint32_t i = 0; i < DIFF; ++i)
                REL::safe_write(funcBase.address() + BRANCH_OFF + BRANCH_SIZE + i, NOP);
        }
    };

    bool PatchEquipShoutEventSpam()
    {
        logger::trace("- equip shout event spam fix - "sv);

        EquipShoutEventSpamPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class GetKeywordItemCountPatch
    {
    public:
        static void Install()
        {
            const auto command = RE::SCRIPT_FUNCTION::LocateScriptCommand(LONG_NAME);
            if (command)
            {
                command->executeFunction = Execute;
                command->conditionFunction = Eval;
            }
        }

    private:
        static bool Execute(const RE::SCRIPT_PARAMETER*, RE::SCRIPT_FUNCTION::ScriptData*, RE::TESObjectREFR* a_thisObj, RE::TESObjectREFR*, RE::Script* a_scriptObj, RE::ScriptLocals*, double& a_result, std::uint32_t&)
        {
            if (!a_scriptObj || a_scriptObj->refObjects.empty())
            {
                return false;
            }

            auto param = a_scriptObj->refObjects.front();
            if (!param->form || param->form->IsNot(RE::FormType::Keyword))
            {
                return false;
            }

            return Eval(a_thisObj, param->form, 0, a_result);
        }

        static bool Eval(RE::TESObjectREFR* a_thisObj, void* a_param1, [[maybe_unused]] void* a_param2, double& a_result)
        {
            a_result = 0.0;
            if (!a_thisObj || !a_param1)
            {
                return true;
            }

            const auto log = RE::ConsoleLog::GetSingleton();
            if (!a_thisObj->GetContainer())
            {
                if (log && log->IsConsoleMode())
                {
                    log->Print("Calling Reference is not a Container Object");
                }
                return true;
            }

            const auto keyword = static_cast<RE::BGSKeyword*>(a_param1);
            const auto inv = a_thisObj->GetInventoryCounts([&](RE::TESBoundObject& a_object) -> bool {
                auto keywordForm = a_object.As<RE::BGSKeywordForm>();
                return keywordForm && keywordForm->HasKeyword(keyword);
            });

            for (const auto& elem : inv)
            {
                a_result += elem.second;
            }

            if (log && log->IsConsoleMode())
            {
                log->Print("GetKeywordItemCount >> %0.2f", a_result);
            }

            return true;
        }

        static constexpr char LONG_NAME[] = "GetKeywordItemCount";
    };

    bool PatchGetKeywordItemCount()
    {
        logger::trace("- broken GetKeywordItemCount condition function fix -"sv);

        GetKeywordItemCountPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class GHeapLeakDetectionCrashPatch
    {
    public:
        static void Install()
        {
            constexpr std::size_t START = 0x4B;
            constexpr std::size_t END = 0x5C;
            constexpr std::uint8_t NOP = 0x90;

            REL::Relocation<std::uintptr_t> funcBase{ REL::ID(85757) };
            for (std::size_t i = START; i < END; ++i)
                REL::safe_write(funcBase.address() + i, NOP);
        }
    };

    bool PatchGHeapLeakDetectionCrash()
    {
        logger::trace("- GHeap leak detection crash fix -"sv);

        GHeapLeakDetectionCrashPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class LipSyncPatch
    {
    public:
        static void Install()
        {
            constexpr std::array offsets{
                0x1E,
                0x3A,
                0x9A,
                0xD8
            };

            constexpr std::uint8_t JMP = 0xEB;

            REL::Relocation<std::uintptr_t> funcBase{ REL::ID(16023) };
            for (auto& offset : offsets)
                REL::safe_write(funcBase.address() + offset, JMP);  // jns -> jmp
        }
    };

    bool PatchLipSync()
    {
        logger::trace("- lip sync bug fix -"sv);

        LipSyncPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class MemoryAccessErrorsPatch
    {
    public:
        static void Install()
        {
            PatchSnowMaterialCase();
            PatchShaderParticleGeometryDataLimit();
            PatchUseAfterFree();
        }

    private:
        static void PatchSnowMaterialCase()
        {
            logger::trace("patching BSLightingShader::SetupMaterial snow material case"sv);

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

            REL::Relocation<std::uintptr_t> vtbl{ REL::ID(304565) };
            REL::Relocation<std::uintptr_t> funcBase{ REL::ID(100563) };
            std::uintptr_t hook = funcBase.address() + 0x4E0;
            std::uintptr_t exit = funcBase.address() + 0x5B6;
            Patch patch(vtbl.address(), hook, exit);
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                hook,
                trampoline.allocate(patch));
        }

        static void PatchShaderParticleGeometryDataLimit()
        {
            logger::trace("patching BGSShaderParticleGeometryData limit"sv);

            REL::Relocation<std::uintptr_t> vtbl{ REL::ID(234671) };
            _Load = vtbl.write_vfunc(0x6, Load);
        }

        static void PatchUseAfterFree()
        {
            logger::trace("patching BSShadowDirectionalLight use after free"sv);

            // Xbyak is used here to generate the ASM to use instead of just doing it by hand
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

            REL::Relocation<std::uintptr_t> target{ REL::ID(101499) };
            REL::safe_write(target.address() + 0x1AFD, stl::span{ patch.getCode(), patch.getSize() });
        }

        // BGSShaderParticleGeometryData::Load
        static bool Load(RE::BGSShaderParticleGeometryData* a_this, RE::TESFile* a_file)
        {
            const bool retVal = _Load(a_this, a_file);

            // the game doesn't allow more than 10 here
            if (a_this->data.size() >= 12)
            {
                const auto particleDensity = a_this->data[11];
                if (particleDensity.f > 10.0)
                    a_this->data[11].f = 10.0f;
            }

            return retVal;
        }

        inline static REL::Relocation<decltype(&RE::BGSShaderParticleGeometryData::Load)> _Load;
    };

    bool PatchMemoryAccessErrors()
    {
        logger::trace("- memory access errors fix -"sv);

        MemoryAccessErrorsPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class MusicOverlapPatch
    {
    public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> target(REL::ID{ 20700 });
            REL::safe_write(target.address() + 0x22, std::uint16_t(0x9090));
        }
    };

    bool PatchMusicOverlap()
    {
        logger::trace("- music overlap fix -"sv);

        MusicOverlapPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class MO5STypoPatch
    {
    public:
        static void Install()
        {
            constexpr std::uint8_t BYTE = 0x35;

            // Change "D" to "5"
            REL::Relocation<std::uintptr_t> typo{ REL::ID(14653) };
            REL::safe_write(typo.address() + 0x83, BYTE);
        }
    };

    bool PatchMO5STypo()
    {
        logger::trace("- MO5S Typo fix -"sv);

        MO5STypoPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class NullProcessCrashPatch
    {
    public:
        static void Install()
        {
            auto& trampoline = SKSE::GetTrampoline();

            {
                REL::Relocation<std::uintptr_t> funcBase{ REL::ID(37943) };
                trampoline.write_call<5>(funcBase.address() + 0x6C, GetEquippedLeftHand);
                trampoline.write_call<5>(funcBase.address() + 0x9C, GetEquippedRightHand);
            }

            {
                REL::Relocation<std::uintptr_t> funcBase{ REL::ID(46074) };
                trampoline.write_call<5>(funcBase.address() + 0x47, GetEquippedLeftHand);
                trampoline.write_call<5>(funcBase.address() + 0x56, GetEquippedRightHand);
            }
        }

    private:
        static RE::TESForm* GetEquippedLeftHand(RE::AIProcess* a_process)
        {
            return a_process ? a_process->GetEquippedLeftHand() : nullptr;
        }

        static RE::TESForm* GetEquippedRightHand(RE::AIProcess* a_process)
        {
            return a_process ? a_process->GetEquippedRightHand() : nullptr;
        }
    };

    bool PatchNullProcessCrash()
    {
        logger::trace("- null process crash fix -"sv);

        NullProcessCrashPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class PerkFragmentIsRunningPatch
    {
    public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> funcBase{ REL::ID(21119) };
            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_call<5>(funcBase.address() + 0x22, IsRunning);
        }

    private:
        static bool IsRunning(RE::Actor* a_this)
        {
            return a_this ? a_this->IsRunning() : false;
        }
    };

    bool PatchPerkFragmentIsRunning()
    {
        logger::trace("- perk fragment IsRunning fix -"sv);

        PerkFragmentIsRunningPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class RemovedSpellBookPatch
    {
    public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> vtbl{ REL::ID(234122) };
            _LoadGame = vtbl.write_vfunc(0xF, LoadGame);
        }

    private:
        static void LoadGame(RE::TESObjectBOOK* a_this, RE::BGSLoadFormBuffer* a_buf)
        {
            using Flag = RE::OBJ_BOOK::Flag;

            _LoadGame(a_this, a_buf);

            if (a_this->data.teaches.actorValueToAdvance == RE::ActorValue::kNone)
            {
                if (a_this->TeachesSkill())
                {
                    a_this->data.flags.reset(Flag::kAdvancesActorValue);
                }

                if (a_this->TeachesSpell())
                {
                    a_this->data.flags.reset(Flag::kTeachesSpell);
                }
            }
        }

        static inline REL::Relocation<decltype(&RE::TESObjectBOOK::LoadGame)> _LoadGame;  // 0xF
    };

    bool PatchRemovedSpellBook()
    {
        logger::trace("- removed spell book fix -"sv);

        RemovedSpellBookPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class SlowTimeCameraMovementPatch
    {
    public:
        static void Install()
        {
            logger::trace("patching camera movement to use frame timer that ignores slow time"sv);

            constexpr std::array targets{
                std::make_pair(49977, 0x2F),
                std::make_pair(49977, 0x96),
                std::make_pair(49977, 0x1FD),
                std::make_pair(49980, 0xBA),
                std::make_pair(49981, 0x17)
            };

            for (const auto& [id, offset] : targets)
            {
                REL::Relocation<std::int16_t*> target{ REL::ID(id), offset };
                REL::safe_write<std::uint16_t>(target.address(), *target + 0x4);
            }
        }
    };

    bool PatchSlowTimeCameraMovement()
    {
        logger::trace("- slow time camera movement fix -"sv);

        SlowTimeCameraMovementPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class TorchLandscapePatch
    {
    public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> target{ REL::ID(17208), 0x52D };

            struct Patch : Xbyak::CodeGenerator
            {
                Patch(std::uintptr_t a_func)
                {
                    Xbyak::Label f;

                    mov(r9, rdi);
                    jmp(ptr[rip + f]);

                    L(f);
                    dq(a_func);
                }
            };

            Patch patch(reinterpret_cast<std::uintptr_t>(AddLight));
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            _AddLight = trampoline.write_call<5>(
                target.address(),
                trampoline.allocate(patch));
        }

    private:
        static RE::BSLight* AddLight(
            RE::ShadowSceneNode* a_this,
            RE::NiLight* a_list,
            RE::ShadowSceneNode::LIGHT_CREATE_PARAMS& a_params,
            not_null<RE::TESObjectREFR*> a_requester)
        {
            if (a_requester->Is(RE::FormType::ActorCharacter))
            {
                a_params.affectLand = true;
            }

            return _AddLight(a_this, a_list, a_params);
        }

        static inline REL::Relocation<RE::BSLight*(RE::ShadowSceneNode*, RE::NiLight*, const RE::ShadowSceneNode::LIGHT_CREATE_PARAMS&)> _AddLight;
    };

    bool PatchTorchLandscape()
    {
        logger::trace("- torch landscape fix -"sv);

        TorchLandscapePatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class TreeReflectionsPatch
    {
    public:
        static bool Install()
        {
            const auto handle = GetModuleHandleA("d3dcompiler_46e.dll");

            if (handle)
            {
                logger::trace("enb detected - disabling fix, please use ENB's tree reflection fix instead"sv);
                return true;
            }

            logger::trace("patching BSDistantTreeShader vfunc 3"sv);
            struct Patch : Xbyak::CodeGenerator
            {
                Patch(std::uintptr_t a_target)
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
                    dq(a_target + 0x6);
                }
            };

            REL::Relocation<std::uintptr_t> target{ REL::ID(100771), 0x37 };
            Patch patch(target.address());
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                target.address(),
                trampoline.allocate(patch));

            logger::trace("success"sv);

            return true;
        }
    };

    bool PatchTreeReflections()
    {
        logger::trace("- blocky tree reflections fix -"sv);

        return TreeReflectionsPatch::Install();
    }

    class VerticalLookSensitivityPatch
    {
    public:
        static void Install()
        {
            PatchThirdPersonStateHook();
            PatchDragonCameraStateHook();
            PatchHorseCameraStateHook();
        }

    private:
        static void PatchThirdPersonStateHook()
        {
            logger::trace("patching third person state..."sv);

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
                    mov(r8, ptr[rip + magicLabel]);
                    movss(xmm4, dword[r8]);
                    //.text:0000000140850D89                 movaps  xmm3, xmm4
                    // use timer
                    mov(r8, ptr[rip + timerLabel]);
                    movss(xmm3, dword[r8]);

                    // exit 850D8C
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_hookTarget + 0xB);

                    L(magicLabel);
                    dq(uintptr_t(&MAGIC));

                    L(timerLabel);
                    dq(a_frameTimer);
                }
            };

            REL::Relocation<std::uintptr_t> hookTarget{ REL::ID(49978), 0x71 };
            REL::Relocation<float*> noSlowFrameTimer{ REL::ID(523661) };
            Patch patch(hookTarget.address(), noSlowFrameTimer.address());
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                hookTarget.address(),
                trampoline.allocate(patch));

            logger::trace("success"sv);
        }

        static void PatchDragonCameraStateHook()
        {
            logger::trace("patching dragon camera state..."sv);

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
                    mov(r8, ptr[rip + magicLabel]);
                    movss(xmm4, dword[r8]);
                    //.text:0000000140850D89                 movaps  xmm3, xmm4
                    // use timer
                    mov(r8, ptr[rip + timerLabel]);
                    movss(xmm3, dword[r8]);

                    // exit 850D8C
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_hookTarget + 0xB);

                    L(magicLabel);
                    dq(uintptr_t(&MAGIC));

                    L(timerLabel);
                    dq(a_frameTimer);
                }
            };

            REL::Relocation<std::uintptr_t> hookTarget{ REL::ID(32370), 0x5F };
            REL::Relocation<float*> noSlowFrameTimer{ REL::ID(523661) };
            Patch patch(hookTarget.address(), noSlowFrameTimer.address());
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                hookTarget.address(),
                trampoline.allocate(patch));

            logger::trace("success"sv);
        }

        static void PatchHorseCameraStateHook()
        {
            logger::trace("patching horse camera state..."sv);

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
                    mov(r8, ptr[rip + magicLabel]);
                    movss(xmm4, dword[r8]);
                    //.text:0000000140850D89                 movaps  xmm3, xmm4
                    // use timer
                    mov(r8, ptr[rip + timerLabel]);
                    movss(xmm3, dword[r8]);

                    // exit 850D8C
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(a_hookTarget + 0xB);

                    L(magicLabel);
                    dq(uintptr_t(&MAGIC));

                    L(timerLabel);
                    dq(a_frameTimer);
                }
            };

            REL::Relocation<std::uintptr_t> hookTarget{ REL::ID(49839), 0x5F };
            REL::Relocation<float*> noSlowFrameTimer{ REL::ID(523661) };
            Patch patch(hookTarget.address(), noSlowFrameTimer.address());
            patch.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                hookTarget.address(),
                trampoline.allocate(patch));

            logger::trace("success"sv);
        }

        static inline std::int32_t MAGIC = 0x3CC0C0C0;  // 1 / 42.5
    };

    bool PatchVerticalLookSensitivity()
    {
        logger::trace("- vertical look sensitivity fix -"sv);

        VerticalLookSensitivityPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    class WeaponBlockScalingPatch
    {
    public:
        static void Install()
        {
            constexpr std::uint8_t NOP = 0x90;
            constexpr std::size_t START = 0x3B8;
            constexpr std::size_t END = 0x3D1;
            constexpr std::size_t CAVE_SIZE = END - START;

            REL::Relocation<std::uintptr_t> target{ REL::ID(42842), START };

            struct Patch : Xbyak::CodeGenerator
            {
                Patch(std::uintptr_t a_func)
                {
                    // rbx = Actor*

                    mov(rcx, rbx);
                    mov(rdx, a_func);
                    call(rdx);
                    movaps(xmm8, xmm0);
                }
            };

            Patch patch(unrestricted_cast<std::uintptr_t>(CalcWeaponDamage));
            patch.ready();
            assert(patch.getSize() <= CAVE_SIZE);

            REL::safe_write(target.address(), stl::span{ patch.getCode(), patch.getSize() });

            for (std::size_t i = patch.getSize(); i < CAVE_SIZE; ++i)
                REL::safe_write(target.address() + i, NOP);
        }

    private:
        static float CalcWeaponDamage(RE::Actor* a_target)
        {
            auto weap = GetWeaponData(a_target);
            if (weap)
                return static_cast<float>(weap->GetAttackDamage());
            else
                return 0.0F;
        }

        static RE::TESObjectWEAP* GetWeaponData(RE::Actor* a_actor)
        {
            const auto proc = a_actor->currentProcess;
            if (!proc || !proc->middleHigh)
            {
                return nullptr;
            }

            const auto middleProc = proc->middleHigh;
            const std::array entries{
                middleProc->bothHands,
                middleProc->rightHand,
                middleProc->leftHand
            };

            for (const auto& entry : entries)
            {
                if (entry)
                {
                    const auto obj = entry->GetObject();
                    if (obj && obj->Is(RE::FormType::Weapon))
                    {
                        return static_cast<RE::TESObjectWEAP*>(obj);
                    }
                }
            }

            return nullptr;
        }
    };

    bool PatchWeaponBlockScaling()
    {
        logger::trace("- weapon block scaling fix -"sv);

        WeaponBlockScalingPatch::Install();

        logger::trace("success"sv);
        return true;
    }

    bool PatchShadowSceneNodeNullptrCrash()
    {
        // written for 1.5.97 but should be compatible

        // sub_1412BACA0
        const REL::ID fid(99708);
        const uint64_t faddr = fid.address();
        logger::trace(FMT_STRING("- workaround for crash in ShadowSceneNode::unk_{:X} -"), fid.offset());

        const uint8_t *crashaddr = (uint8_t*)(uintptr_t)(faddr + 22);
        /*
                        mov     rax,qword ptr [rdx]
        ... some movs ...
        FF 50 18        call    qword ptr [rax+18h] // <--- crashes here because rax == 0
        84 C0           test    al, al
        74 ??           jz      short loc_1412BACD3
        */

        static const uint8_t expected[] = { 0xFF, 0x50, 0x18, 0x84, 0xC0, 0x74 };
        if(std::memcmp((const void*)crashaddr, expected, sizeof(expected)))
        {
            logger::trace("Code is not as expected, skipping patch"sv);
            return false;
        }

        const int8_t disp8 = crashaddr[sizeof(expected)]; // jz short displacement (should be 0x16)
        const uint8_t *jmpdstZero    = crashaddr + sizeof(expected) + disp8 + 1;
        const uint8_t *jmpdstNonZero = crashaddr + sizeof(expected) + 1;

        struct Code : Xbyak::CodeGenerator
        {
            Code(std::uintptr_t contNonzeroAddr, std::uintptr_t contZeroAddr)
            {
                Xbyak::Label contAddrLbl, zeroLbl, zeroAddrLbl;

                // check for NULL
                test(rax, rax);
                jz(zeroLbl);

                // original instructions minus jz short
                db(expected, sizeof(expected) - 1);

                jz(zeroLbl);
                jmp(ptr[rip + contAddrLbl]);
                L(zeroLbl);
                jmp(ptr[rip + zeroAddrLbl]);

                L(contAddrLbl);
                dq(contNonzeroAddr);
                L(zeroAddrLbl);
                dq(contZeroAddr);
            }
        };

        Code code(unrestricted_cast<std::uintptr_t>(jmpdstNonZero), unrestricted_cast<std::uintptr_t>(jmpdstZero));
        code.ready();

        logger::trace("installing fix"sv);
        auto& trampoline = SKSE::GetTrampoline();
        if (!trampoline.write_branch<5>(unrestricted_cast<std::uintptr_t>(crashaddr), trampoline.allocate(code)))
            return false;

        logger::trace("success"sv);
        return true;
    }

    bool PatchFaceGenMorphDataHeadNullptrCrash()
    {
        // written for 1.5.97 but should be compatible

        logger::trace("- fix for crash in FaceGenMorphDataHead -"sv);
        // loc_1403D68E0
        /*
        .....
        .text:00000001403D690D        xor     r15d, r15d
        .text:00000001403D6910        test    rdx, rdx           // rdx == 0 causes the first crash in a few instructions
        .text:00000001403D6913    +-- jz      short loc_1403D6929
        .text:00000001403D6915    |   lea     rdx, unk_142F07D08 // <-- rdx != 0, this is the good case and
        .text:00000001403D691C    |   mov     rcx, rax           //      we want to continue here if everything is well
        .text:00000001403D691F    |   call    sub_140C60AD0
        .text:00000001403D6924    |   mov     rbx, rax         // <--- This sometimes has rax == 0 -> second possibility how rbx ends up as 0
        .text:00000001403D6927  +-|-- jmp     short loc_1403D692C
        .text:00000001403D6929  | +-->mov     rbx, r15         // <--- if this is executed it'll crash on the next line since r15 is 0 here
        .text:00000001403D692C  +---> mov     eax, [rbx+24h]
        .text:00000001403D692F        comiss  xmm6, cs:dword_14152259C
        .text:00000001403D6936        jb      loc_1403D6D1F  // ---> This jumps to the function stack restore + return point
        */
        // What can we do here? There are two ways for rbx to end up NULL,
        // so we need to move the last mov followed by comiss into the trampoline
        // and wrap that into a check for rbx != 0.
        // The 'mov rbx, r15' is also weird. Only the lower 32 bits of r15 are guaranteed to be 0,
        // but then rbx is used as an address. So it's probably better to replace that instruction
        // with 'xor ebx, ebx' to make sure our inserted check will catch it in all cases.

        // TODO: There is probably a cleaner way to patch this without moving a float constant into the trampoline, but this is tested and works ok

        const uint64_t faddr = REL::ID(26343).address();

        // check that test rdx,rdx and jz short are there
        uint8_t *testrdx = (uint8_t*)(uintptr_t)(faddr + 0x30);
        static const uint8_t expectedTestRdxJz[] = { 0x48, 0x85, 0xD2, 0x74 };
        if(std::memcmp(testrdx, expectedTestRdxJz, sizeof(expectedTestRdxJz)))
            return false;

        // Where does the jz short go?
        const int8_t disp8 = testrdx[sizeof(expectedTestRdxJz)]; // jz short displacement (should be 0x14)
              uint8_t *jmpdstZero = testrdx + sizeof(expectedTestRdxJz) + disp8 + 1;
        const uint8_t *jmpdstNonZero = testrdx + sizeof(expectedTestRdxJz) + 1; // success case

        // Is the zero case jump target 'mov rbx, r15'? (This is patched later)
        static const uint8_t expectedMovRbx[] = { 0x49, 0x8B, 0xDF };
        if(std::memcmp(jmpdstZero, expectedMovRbx, sizeof(expectedMovRbx)))
            return false;

        // Find the where to insert the trampoline jump and where the jump to the exit is
        uint8_t *pUnsafeMov = jmpdstZero + sizeof(expectedMovRbx); // this is the mov that potentially causes a crash
        if(pUnsafeMov[0] != 0x8B)
            return false;

        static const uint8_t copysize = 3; // mov -- can't just copy the comiss because it uses a relative address :<
        const uint8_t *jbExit = pUnsafeMov + copysize + 7; // + comiss

        // Make sure jb => exit is there as expected
        static const uint8_t expectedJB[] = { 0x0f, 0x82 };
        if(std::memcmp(jbExit, expectedJB, sizeof(expectedJB)))
            return false;

        // Get the jump offset
        int32_t jbOffs = *(int32_t*)(jbExit + sizeof(expectedJB));

        // Figure out where the function exit location is
        const uint8_t *nextInst = jbExit + 2 + sizeof(int32_t);
        const uint8_t *exitLoc = nextInst + jbOffs;

        struct Code : Xbyak::CodeGenerator
        {
            Code(const uint8_t *originalcode, std::uintptr_t pContLoc, std::uintptr_t pExitLoc) : Xbyak::CodeGenerator()
            {
                Xbyak::Label zeroLbl, fltConstL;

                test(rbx, rbx);
                jz(zeroLbl);

                db(originalcode, copysize); // the unsafe mov
                comiss(xmm6, ptr[rip + fltConstL]);

                jmp(ptr[rip]);
                dq(pContLoc);

                L(zeroLbl);
                jmp(ptr[rip]);
                dq(pExitLoc);

                L(fltConstL);
                dd(0x0BF800000); // -1.0f
            }
        };

        Code code(pUnsafeMov, unrestricted_cast<std::uintptr_t>(jbExit), unrestricted_cast<std::uintptr_t>(exitLoc));
        code.ready();

        logger::trace("installing fix"sv);
        auto& trampoline = SKSE::GetTrampoline();
        if (!trampoline.write_branch<5>(unrestricted_cast<std::uintptr_t>(pUnsafeMov), trampoline.allocate(code)))
            return false;

        // nop out the rest of the original code so that the diasm/debugger is less confused
        static const uint8_t nop5[] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
        REL::safe_write(uintptr_t(pUnsafeMov) + 5, nop5, sizeof(nop5));

        // Fix rbx clear (3 bytes to cover)
        struct Patch : Xbyak::CodeGenerator
        {
            Patch()
            {
                xor_(ebx, ebx); // 2 bytes
                nop();          // 1 byte
            }
        };
        Patch patch;
        patch.ready();
        assert(patch.getSize() == 3);
        REL::safe_write((uintptr_t)jmpdstZero, patch.getCode(), 3);

        logger::trace("success"sv);
        return true;
    }

    // Fixes crash on weapon swing impact when the weapon loses some associated data just before HitData::InitializeHitData() is called.
    // Possibly a disarm caused by an on-hit triggered perk or spell effect?
    bool PatchInitializeHitDataNullptrCrash()
    {
        /* HitData::InitializeHitData() aka sub_140742850 in version 1.5.97
        ... part of the function setup ...
        00007FF74208285E 4D 8B F9                  mov  r15,r9  <---- This is weapon. fine if it's NULL
        00007FF742082861 49 8B E8                  mov  rbp,r8
        00007FF742082864 48 8B FA                  mov  rdi,rdx
        00007FF742082867 48 8B D9                  mov  rbx,rcx
        [...]
        00007FF7420828A9 4D 85 FF                  test r15,r15
        00007FF7420828AC 74 09                 +-- je   00007FF7420828B7
        00007FF7420828AE 49 8B 0F              |   mov  rcx,qword ptr [r15]    <--- This is weapon->object and may load rcx == 0
        00007FF7420828B1 80 79 1A 29           |   cmp  byte ptr [rcx+1Ah],29h <--- Crashes here when rcx == 0
        00007FF7420828B5 74 07                 |   je   00007FF7420828BE
        00007FF7420828B7 48 8B 0D AA CF 7B 02  +-> mov  rcx,qword ptr [7FF74483F868h]

        The code checks for r15 != NULL but forgets to check the loaded pointer before dereferencing.
        Patch near the entry point of the function to make sure nothing similar happens further down somewhere.
        The fix is equivalent to inserting this at the start of the function (thanks Nukem):
          if (weapon && !weapon->object)
              weapon = nullptr;
        */

        logger::trace("- fix for crash in HitData::InitializeHitData -"sv);

        const uint64_t faddr = REL::ID(42832).address();
        const uint8_t *locMovR15 = unrestricted_cast<const uint8_t*>(faddr + 14);
        //                        mov r15,r9          mov rbp,r8 (replicated in the trampoline code below)
        const char expected[] = { 0x4D, 0x8B, 0xF9,   0x49, 0x8B, 0xE8 };
        if(std::memcmp(locMovR15, expected, sizeof(expected)))
            return false;

        const uint8_t *locNext = locMovR15 + sizeof(expected);

        struct Patch : Xbyak::CodeGenerator
        {
            Patch(uintptr_t continueAt)
            {
                Xbyak::Label out;
                // At the point where this is injected we're still moving function parameters into the correct registers.
                // The function uses weapon as r15, but it's passed to the function in r9.
                // So we clear r15 and only move r9 there if it's safe to do so.
                xor_(r15, r15);
                test(r9, r9);
                jz(out);
                mov(rbx, qword[r9]); // rbx is free to clobber at this point since there was a 'push rbx' before
                test(rbx, rbx);
                cmovnz(r15, r9); // keep weapon only if weapon->object != NULL
                L(out);
                mov(rbp, r8); // Restore this from where the trampoline jump was placed
                jmp(ptr[rip]);
                dq(continueAt);
            }
        };
        Patch code(unrestricted_cast<uintptr_t>(locNext));
        code.ready();

        logger::trace("installing fix"sv);
        auto& trampoline = SKSE::GetTrampoline();
        if (!trampoline.write_branch<5>(unrestricted_cast<uintptr_t>(locMovR15), trampoline.allocate(code)))
            return false;

        // The trampoline jump needs 5 bytes but the 2 patched instructions are 6 bytes long so there's 1 byte left.
        // This byte is never executed, but in order to make disassemblers happy make it a nop.
        REL::safe_fill(unrestricted_cast<uintptr_t>(locMovR15 + 5), 0x90, sizeof(expected) - 5);

        logger::trace("success"sv);
        return true;
    }

}

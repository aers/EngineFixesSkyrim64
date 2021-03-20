#include "patches.h"

namespace patches
{
    REL::Relocation<float*> FrameTimer_WithSlowTime{ g_FrameTimer_SlowTime_offset };

    // +0x252
    REL::Relocation<std::uintptr_t> GameLoop_Hook{ GameLoop_Hook_offset, 0x252 };
    REL::Relocation<std::uint32_t*> UnkGameLoopDword{ UnkGameLoopDword_offset };

    // 5th function in??_7BSWaterShader@@6B@ vtbl
    // F3 0F 10 0D ? ? ? ? F3 0F 11 4C 82 ?
    // loads TIMER_DEFAULT which is a timer representing the GameHour in seconds
    REL::Relocation<std::uintptr_t> WaterShader_ReadTimer_Hook{ WaterShader_ReadTimer_Hook_offset, 0x4A9 };

    float timer = 8 * 3600;  // Game timer inits to 8 AM

    void update_timer()
    {
        timer = timer + *FrameTimer_WithSlowTime * static_cast<float>(*config::waterflowSpeed);
        if (timer > 86400)  // reset timer to 0 if we go past 24 hours
            timer = timer - 86400;
    }

    bool PatchWaterflowAnimation()
    {
        logger::trace("- waterflow timer -"sv);

        logger::trace("hooking new timer to the game update loop..."sv);
        {
            struct GameLoopHook_Code : Xbyak::CodeGenerator
            {
                GameLoopHook_Code()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label funcLabel;
                    Xbyak::Label unkDwordLabel;

                    // enter 5B36F2
                    // some people were crashing and while I'm pretty sure the registers dont need to be saved looking at the function in question, this was just a check to see if that was the problem
                    // (it wasnt)
                    /*
                    sub(rsp, 0x20);
                    vmovdqu(ptr[rsp], xmm0);
                    vmovdqu(ptr[rsp + 0x10], xmm1);
                    push(rax);*/
                    sub(rsp, 0x20);
                    call(ptr[rip + funcLabel]);
                    add(rsp, 0x20);
                    /*
                    pop(rax);
                    vmovdqu(xmm1, ptr[rsp + 0x10]);
                    vmovdqu(xmm0, ptr[rsp]);
                    add(rsp, 0x20);*/
                    // .text:00000001405B36F2                 mov     edx, cs : dword_142F92950
                    mov(rdx, ptr[rip + unkDwordLabel]);
                    mov(edx, dword[rdx]);

                    // exit 5B36F8
                    jmp(ptr[rip + retnLabel]);

                    L(funcLabel);
                    dq(uintptr_t(update_timer));

                    L(retnLabel);
                    dq(GameLoop_Hook.address() + 0x6);

                    L(unkDwordLabel);
                    dq(UnkGameLoopDword.address());
                }
            };

            GameLoopHook_Code code;
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(GameLoop_Hook.address(), trampoline.allocate(code));
        }
        logger::trace("replacing water flow timer with our timer..."sv);
        {
            struct WaterFlowHook_Code : Xbyak::CodeGenerator
            {
                WaterFlowHook_Code()
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
                    dq(WaterShader_ReadTimer_Hook.address() + 0xE);

                    L(timerLabel);
                    dq(std::uintptr_t(&timer));
                }
            };

            WaterFlowHook_Code code;
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                WaterShader_ReadTimer_Hook.address(),
                trampoline.allocate(code));
        }
        logger::trace("success"sv);

        return true;
    }

    decltype(&fopen_s) VC140_fopen_s;
    errno_t hk_fopen_s(FILE** File, const char* Filename, const char* Mode)
    {
        errno_t err = VC140_fopen_s(File, Filename, Mode);

        if (err != 0)
            logger::warn(FMT_STRING("Error occurred trying to open file: fopen_s({}, {}), errno {}"), Filename, Mode, err);

        return err;
    }

    decltype(&_wfopen_s) VC140_wfopen_s;
    errno_t hk_wfopen_s(FILE** File, const wchar_t* Filename, const wchar_t* Mode)
    {
        errno_t err = VC140_wfopen_s(File, Filename, Mode);

        if (err != 0)
            logger::warn(FMT_STRING("Error occurred trying to open file: _wfopen_s(?, ?), errno {}"), /*Filename, Mode,*/ err);

        return err;
    }

    decltype(&fopen) VC140_fopen;
    FILE* hk_fopen(const char* Filename, const char* Mode)
    {
        FILE* f = VC140_fopen(Filename, Mode);

        if (!f)
            logger::warn(FMT_STRING("Error occurred trying to open file: fopen({}, {})"), Filename, Mode);

        return f;
    }

    bool PatchMaxStdio()
    {
        logger::trace("- max stdio -"sv);

        const auto handle = GetModuleHandle(L"API-MS-WIN-CRT-STDIO-L1-1-0.DLL");
        const auto proc = handle ?
            reinterpret_cast<decltype(&_setmaxstdio)>(GetProcAddress(handle, "_setmaxstdio")) : nullptr;

        if (!proc) {
            logger::trace("crt stdio module not found, failed"sv);
            return false;
        }

        const auto result = proc(static_cast<int>(*config::patchMaxStdio));
        logger::trace("max stdio set to {}"sv, result);

        *(void**)&VC140_fopen_s = (std::uintptr_t*)SKSE::PatchIAT(hk_fopen_s, "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "fopen_s");
        *(void**)&VC140_wfopen_s = (std::uintptr_t*)SKSE::PatchIAT(hk_wfopen_s, "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "_wfopen_s");
        *(void**)&VC140_fopen = (std::uintptr_t*)SKSE::PatchIAT(hk_fopen, "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "fopen");

        return true;
    }

    REL::Relocation<std::uintptr_t> QuickSaveLoadHandler_HandleEvent_SaveType{ QuickSaveLoadHandler_HandleEvent_SaveType_offset, 0x68 };
    REL::Relocation<std::uintptr_t> QuickSaveLoadHandler_HandleEvent_LoadType{ QuickSaveLoadHandler_HandleEvent_LoadType_offset, 0x9B };

    bool PatchRegularQuicksaves()
    {
        const std::uint32_t regular_save = 0xF0000080;
        const std::uint32_t load_last_save = 0xD0000100;

        logger::trace("- regular quicksaves -"sv);
        REL::safe_write(QuickSaveLoadHandler_HandleEvent_SaveType.address(), regular_save);
        REL::safe_write(QuickSaveLoadHandler_HandleEvent_LoadType.address(), load_last_save);
        logger::trace("success"sv);
        return true;
    }

    REL::Relocation<std::uintptr_t> AchievementModsEnabledFunction{ AchievementModsEnabledFunction_offset };

    bool PatchEnableAchievementsWithMods()
    {
        logger::trace("- enable achievements with mods -"sv);
        // Xbyak is used here to generate the ASM to use instead of just doing it by hand
        struct Patch : Xbyak::CodeGenerator
        {
            Patch()
            {
                mov(al, 0);
                ret();
            }
        };

        Patch patch;
        patch.ready();
        REL::safe_write(AchievementModsEnabledFunction.address(), stl::span{ patch.getCode(), patch.getSize() });

        logger::trace("success"sv);
        return true;
    }

    class DisableChargenPrecachePatch
    {
    public:
        static void Install()
        {
            constexpr std::array ids = {
                static_cast<std::uint64_t>(51507),
                static_cast<std::uint64_t>(51509),
            };

            constexpr std::uint8_t RET = 0xC3;

            for (const auto& id : ids)
            {
                REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
                REL::safe_write(target.address(), RET);
            }
        }
    };

    bool PatchDisableChargenPrecache()
    {
        logger::trace("- disable chargen precache patch -"sv);

        DisableChargenPrecachePatch::Install();

        logger::trace("success"sv);
        return true;
    }

    bool loadSet = false;

    bool hk_TESFile_IsMaster(RE::TESFile* modInfo)
    {
        if (loadSet)
            return true;

        std::uintptr_t returnAddr = (std::uintptr_t)(_ReturnAddress()) - REL::Module::get().base();

        if (returnAddr == 0x16E11E)
        {
            loadSet = true;
            logger::info("load order finished"sv);
            auto dhnl = RE::TESDataHandler::GetSingleton();
            for (auto& mod : dhnl->compiledFileCollection.files)
            {
                mod->recordFlags |= RE::TESFile::RecordFlag::kMaster;
            }
            return true;
        }

        return (modInfo->recordFlags & RE::TESFile::RecordFlag::kMaster) != RE::TESFile::RecordFlag::kNone;
    }

    REL::Relocation<std::uintptr_t> TESFile_IsMaster{ TESFile_IsMaster_offset };

    bool PatchTreatAllModsAsMasters()
    {
        logger::trace("- treat all mods as masters -"sv);
        MessageBoxW(nullptr, L"WARNING: You have the treat all mods as masters patch enabled. I hope you know what you're doing!", L"Engine Fixes for Skyrim Special Edition", MB_OK);
        auto& trampoline = SKSE::GetTrampoline();
        trampoline.write_branch<6>(TESFile_IsMaster.address(), reinterpret_cast<std::uintptr_t>(hk_TESFile_IsMaster));
        logger::trace("success"sv);

        return true;
    }

    REL::Relocation<std::uintptr_t> FirstPersonState_DontSwitchPOV{ FirstPersonState_DontSwitchPOV_offset, 0x43 };
    REL::Relocation<std::uintptr_t> ThirdPersonState_DontSwitchPOV{ ThirdPersonState_DontSwitchPOV_offset, 0x1E8 };

    bool PatchScrollingDoesntSwitchPOV()
    {
        logger::trace("- scrolling doesnt switch POV -"sv);
        constexpr std::uint8_t BYTE{ 0xEB };
        REL::safe_write(FirstPersonState_DontSwitchPOV.address(), BYTE);
        REL::safe_write(ThirdPersonState_DontSwitchPOV.address(), BYTE);
        logger::trace("- success -"sv);
        return true;
    }

    bool PatchSleepWaitTime()
    {
        logger::trace("- sleep wait time -"sv);
        {
            constexpr std::uint8_t NOP{ 0x90 };
            REL::Relocation<std::uintptr_t> target{ REL::ID(51614), 0x1CE };

            struct SleepWaitTime_Code : Xbyak::CodeGenerator
            {
                SleepWaitTime_Code(std::uintptr_t a_address, double a_val)
                {
                    static float VAL = static_cast<float>(a_val);

                    push(rax);

                    mov(rax, unrestricted_cast<std::uintptr_t>(std::addressof(VAL)));
                    comiss(xmm0, ptr[rax]);

                    pop(rax);

                    jmp(ptr[rip]);
                    dq(a_address + 0x7);
                }
            };

            SleepWaitTime_Code code(target.address(), *config::sleepWaitTimeModifier);
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                target.address(),
                trampoline.allocate(code));
            REL::safe_write(target.address() + 0x6, NOP);
        }
        logger::trace("success"sv);
        return true;
    }

    REL::Relocation<std::uintptr_t> Win32FileType_CopyToBuffer{ Win32FileType_CopyToBuffer_offset, 0x14 };
    REL::Relocation<std::uintptr_t> Win32FileType_ctor{ Win32FileType_ctor_offset, 0x14E };
    REL::Relocation<std::uintptr_t> ScrapHeap_GetMaxSize{ ScrapHeap_GetMaxSize_offset, 0x4 };

    bool PatchSaveGameMaxSize()
    {
        logger::trace("- save game max size -"sv);
        constexpr std::uint8_t BYTE{ 0x08 };
        REL::safe_write(Win32FileType_CopyToBuffer.address(), BYTE);
        REL::safe_write(Win32FileType_ctor.address(), BYTE);
        REL::safe_write(ScrapHeap_GetMaxSize.address(), BYTE);

        logger::trace("success"sv);
        return true;
    }
}

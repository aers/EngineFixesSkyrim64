#include "patches.h"

namespace patches
{
    REL::Relocation<float*> FrameTimer_WithSlowTime{ offsets::Common::g_SecondsSinceLastFrame_WorldTime };

    REL::Relocation<std::uintptr_t> Main_Update_Hook{ offsets::WaterflowAnimation::Main_Update, 0x26B };
    REL::Relocation<std::uint32_t*> ApplicationRunTime{ offsets::WaterflowAnimation::g_ApplicationRunTime };

    // 5th function in??_7BSWaterShader@@6B@ vtbl
    // F3 0F 10 0D ? ? ? ? F3 0F 11 4C 82 ?
    // loads TIMER_DEFAULT which is a timer representing the GameHour in seconds
    REL::Relocation<std::uintptr_t> WaterShader_ReadTimer_Hook{ offsets::WaterflowAnimation::WaterShader_SetupMaterial, 0x4BC };

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
                    Xbyak::Label applicationRunTimeLabel;

                    sub(rsp, 0x20);
                    call(ptr[rip + funcLabel]);
                    add(rsp, 0x20);

                    // orig code
                    mov(rdx, ptr[rip + applicationRunTimeLabel]);
                    mov(edx, dword[rdx]);

                    jmp(ptr[rip + retnLabel]);

                    L(funcLabel);
                    dq(uintptr_t(update_timer));

                    L(retnLabel);
                    dq(Main_Update_Hook.address() + 0x6);

                    L(applicationRunTimeLabel);
                    dq(ApplicationRunTime.address());
                }
            };

            GameLoopHook_Code code;
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(Main_Update_Hook.address(), trampoline.allocate(code));
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
                    // .text:000000014141CE5C                 movss   xmm0, cs:dword_141EA2E40
                    // .text:000000014141CE64                 movss   dword ptr [r10+rax*4+0Ch], xmm0
                    mov(r9, ptr[rip + timerLabel]);  // r9 is safe to use, unused again until .text:000000014130E13C                 mov     r9, r12
                    movss(xmm0, dword[r9]);
                    movss(dword[r10 + rax * 4 + 0xC], xmm0);

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

        const auto handle = GetModuleHandleW(L"API-MS-WIN-CRT-STDIO-L1-1-0.DLL");
        const auto proc = handle ?
            reinterpret_cast<decltype(&_setmaxstdio)>(GetProcAddress(handle, "_setmaxstdio")) :
            nullptr;

        if (!proc) {
            logger::trace("crt stdio module not found, failed"sv);
            return false;
        }

        if (*config::patchMaxStdio > 8192)
            *config::patchMaxStdio = 8192;

        const auto result = proc(static_cast<int>(*config::patchMaxStdio));
        logger::trace("max stdio set to {}"sv, result);

        *(void**)&VC140_fopen_s = (std::uintptr_t*)SKSE::PatchIAT(hk_fopen_s, "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "fopen_s");
        *(void**)&VC140_wfopen_s = (std::uintptr_t*)SKSE::PatchIAT(hk_wfopen_s, "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "_wfopen_s");
        *(void**)&VC140_fopen = (std::uintptr_t*)SKSE::PatchIAT(hk_fopen, "API-MS-WIN-CRT-STDIO-L1-1-0.DLL", "fopen");

        return true;
    }

    REL::Relocation<std::uintptr_t> QuickSaveLoadHandler_HandleEvent_SaveType{ offsets::RegularQuicksaves::QuickSaveLoadHandler_ProcessButton, 0x68 };
    REL::Relocation<std::uintptr_t> QuickSaveLoadHandler_HandleEvent_LoadType{ offsets::RegularQuicksaves::QuickSaveLoadHandler_ProcessButton, 0x9B };

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

    REL::Relocation<std::uintptr_t> AchievementModsEnabledFunction{ offsets::AchievementsWithMods::AchievementModsEnabledFunction };

    bool PatchEnableAchievementsWithMods()
    {
        if(!REL::make_pattern<"48 83 EC 28 C6 44 24 ?? ??">().match(AchievementModsEnabledFunction.address())) {
            logger::error("AchievementModsEnabled patch can't be activated, invalid address");
            return false;
        }

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
            constexpr std::uint8_t RET = 0xC3;

            for (const auto& offset : offsets::DisableChargenPrecache::todo)
            {
                REL::Relocation<std::uintptr_t> target{ offset };
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

    REL::Relocation<std::uintptr_t> FirstPersonState_DontSwitchPOV{ offsets::ScrollingDoesntSwitchPOV::FirstPersonState_PlayerInputHandler_ProcessButton, 0x43 };
    REL::Relocation<std::uintptr_t> ThirdPersonState_DontSwitchPOV{ offsets::ScrollingDoesntSwitchPOV::ThirdPersonState_PlayerInputHandler_ProcessButton, 0x1F7 };

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
            REL::Relocation<std::uintptr_t> target{ offsets::SleepWaitTime::SleepWaitMenu_vf4, 0x1D0 };

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

    REL::Relocation<std::uintptr_t> Win32FileType_CopyToBuffer{ offsets::SaveGameMaxSize::Win32FileType_CopyToBuffer, 0x1A };
    REL::Relocation<std::uintptr_t> Win32FileType_ctor{ offsets::SaveGameMaxSize::Win32FileType_Ctor, 0x20E };
    REL::Relocation<std::uintptr_t> ScrapHeap_GetMaxSize{ offsets::SaveGameMaxSize::ScrapHeap_GetMaxSize, 0x4 };

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

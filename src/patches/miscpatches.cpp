#include "patches.h"

namespace patches
{
    RelocPtr<float> FrameTimer_WithSlowTime(g_FrameTimer_SlowTime_offset);

    // +0x252
    RelocAddr<uintptr_t> GameLoop_Hook(GameLoop_Hook_offset);
    RelocPtr<uint32_t> UnkGameLoopDword(UnkGameLoopDword_offset);

    // 5th function in??_7BSWaterShader@@6B@ vtbl
    // F3 0F 10 0D ? ? ? ? F3 0F 11 4C 82 ?
    // loads TIMER_DEFAULT which is a timer representing the GameHour in seconds
    RelocAddr<uintptr_t> WaterShader_ReadTimer_Hook(WaterShader_ReadTimer_Hook_offset);

    float timer = 8 * 3600; // Game timer inits to 8 AM

    void update_timer()
    {
        timer = timer + *FrameTimer_WithSlowTime * config::waterflowSpeed;
        if (timer > 86400) // reset timer to 0 if we go past 24 hours
            timer = timer - 86400;
    }

    bool PatchWaterflowAnimation()
    {
        _VMESSAGE("- waterflow timer -");

        _VMESSAGE("hooking new timer to the game update loop...");
        {
            struct GameLoopHook_Code : Xbyak::CodeGenerator
            {
                GameLoopHook_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
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
                    dq(GameLoop_Hook.GetUIntPtr() + 0x6);

                    L(unkDwordLabel);
                    dq(UnkGameLoopDword.GetUIntPtr());
                }
            };

            void *codeBuf = g_localTrampoline.StartAlloc();
            GameLoopHook_Code code(codeBuf);
            g_localTrampoline.EndAlloc(code.getCurr());

            g_branchTrampoline.Write6Branch(GameLoop_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
        }
        _VMESSAGE("replacing water flow timer with our timer...");
        {
            struct WaterFlowHook_Code : Xbyak::CodeGenerator
            {
                WaterFlowHook_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label timerLabel;

                    // enter 130DFD9
                    // .text:000000014130DFD9                 movss   xmm1, cs:TIMER_DEFAULT
                    // .text:000000014130DFE1                 movss   dword ptr[rdx + rax * 4 + 0Ch], xmm1
                    mov(r9, ptr[rip + timerLabel]); // r9 is safe to use, unused again until .text:000000014130E13C                 mov     r9, r12
                    movss(xmm1, dword[r9]);
                    movss(dword[rdx + rax * 4 + 0xC], xmm1);

                    // exit 130DFE7
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(WaterShader_ReadTimer_Hook.GetUIntPtr() + 0xE);

                    L(timerLabel);
                    dq(uintptr_t(&timer));
                }
            };

            void *codeBuf = g_localTrampoline.StartAlloc();
            WaterFlowHook_Code code(codeBuf);
            g_localTrampoline.EndAlloc(code.getCurr());

            g_branchTrampoline.Write6Branch(WaterShader_ReadTimer_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
        }
        _VMESSAGE("success");

        return true;
    }
    

    bool PatchMaxStdio()
    {
        _VMESSAGE("- max stdio -");

        const HMODULE crtStdioModule = GetModuleHandleA("API-MS-WIN-CRT-STDIO-L1-1-0.DLL");

        if (!crtStdioModule)
        {
            _VMESSAGE("crt stdio module not found, failed");
            return false;
        }

        const auto maxStdio = reinterpret_cast<decltype(&_setmaxstdio)>(GetProcAddress(crtStdioModule, "_setmaxstdio"))(2048);

        _VMESSAGE("max stdio set to %d", maxStdio);

        return true;
    }

    RelocAddr<uintptr_t> QuickSaveLoadHandler_HandleEvent_SaveType(quicksaveloadhandler_handleevent_savetype);
    RelocAddr<uintptr_t> QuickSaveLoadHandler_HandleEvent_LoadType(quicksaveloadhandler_handleevent_loadtype);

    bool PatchRegularQuicksaves()
    {
        const uint32_t regular_save = 0xF0000080;
        const uint32_t load_last_save = 0xD0000100;

        _VMESSAGE("- regular quicksaves -");
        SafeWrite32(QuickSaveLoadHandler_HandleEvent_SaveType.GetUIntPtr(), regular_save);
        SafeWrite32(QuickSaveLoadHandler_HandleEvent_LoadType.GetUIntPtr(), load_last_save);
        _VMESSAGE("success");
        return true;
    }
}
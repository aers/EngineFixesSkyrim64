#include "../skse64_common/BranchTrampoline.h"
#include "../xbyak/xbyak.h"

#include "WaterflowTimer.h"

float timer = 8 * 3600; // Game timer inits to 8 AM

RelocPtr<float> TimerStep(off_TIMER_STEP_FLOAT);

void update_timer()
{
	timer = timer + *TimerStep * 20; // default timescale: 20
	if (timer > 86400) // reset timer to 0 if we go past 24 hours
		timer = timer - 86400;
}

RelocAddr<uintptr_t> GameLoop_Hook(off_GAMELOOP_AFTER_UPDATEGAMEHOUR);
RelocPtr<uint32_t> UnkGameLoopDword(off_UNKNOWN_GAMELOOP_DWORD);

RelocAddr<uintptr_t> WaterShader_ReadTimer_Hook(off_WATERSHADER_READTIMER);

bool PatchWaterflowTimer()
{
	gLog.SetSource("WatrFlow");

	_MESSAGE("Patching water flow timer...");

	_MESSAGE("Hooking new timer to the game update loop...");
	{
		struct GameLoopHook_Code : Xbyak::CodeGenerator
		{
			GameLoopHook_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;
				Xbyak::Label funcLabel;
				Xbyak::Label unkDwordLabel;

				// enter 5B36F2
				sub(rsp, 0x20);
				call(ptr[rip + funcLabel]);
				add(rsp, 0x20);
				// .text:00000001405B36F2                 mov     edx, cs : dword_142F92950
				mov(rdx, ptr[rip+unkDwordLabel]);
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
	_MESSAGE("Replacing water flow timer with our timer...");
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
	_MESSAGE("Patched");

	return true;
}
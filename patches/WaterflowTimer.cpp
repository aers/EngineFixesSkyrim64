#include "../skse64_common/BranchTrampoline.h"
#include "../xbyak/xbyak.h"

#include "../config.h"
#include "WaterflowTimer.h"

namespace WaterflowTimer
{
	// rough game timer for realtime (in seconds) passing per frame
	// there are two timers, the first adjusts for slow time to slow the game down during slowtime effects
	// the second always represents frametime
	// other game timers are derived from these ones
	// E8 ? ? ? ? 84 DB 74 24 -> +0x230
	RelocPtr<float> FrameTimer_WithSlowTime(0x02F9294);

	// +0x252
	RelocAddr<uintptr_t> GameLoop_Hook(0x005B36F2);
	RelocPtr<uint32_t> UnkGameLoopDword(0x02F92950);

	// 5th function in??_7BSWaterShader@@6B@ vtbl
	// F3 0F 10 0D ? ? ? ? F3 0F 11 4C 82 ?
	// loads TIMER_DEFAULT which is a timer representing the GameHour in seconds
	RelocAddr<uintptr_t> WaterShader_ReadTimer_Hook(0x0130DFD9);

	float timer = 8 * 3600; // Game timer inits to 8 AM

	void update_timer()
	{
		timer = timer + *FrameTimer_WithSlowTime * config::waterflowTimescale;
		if (timer > 86400) // reset timer to 0 if we go past 24 hours
			timer = timer - 86400;
	}

	bool Patch()
	{
		_MESSAGE("- waterflow timer -");

		_MESSAGE("hooking new timer to the game update loop...");
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
		_MESSAGE("replacing water flow timer with our timer...");
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
		_MESSAGE("success");

		return true;
	}
}
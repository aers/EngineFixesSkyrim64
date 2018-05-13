#include "../../skse64_common/BranchTrampoline.h"
#include "../../skse64_common/Relocation.h"

#include "../../xbyak/xbyak.h"

namespace VerticalLookSensitivity
{
	static int magic = 0x3CC0C0C0; // 1 / 42.5

	// WaterflowTimer's Frame Timer +0x4
	RelocPtr<float> FrameTimer_WithoutSlowTime(0x02F9294C);

	// ??_7ThirdPersonState@@6B@ vtbl last function + 0x71
	RelocAddr<uintptr_t> ThirdPersonState_Vfunc_Hook(0x00850D81);
	// ??_7DragonCameraState@@6B@ vtbl last function + 0x5F
	RelocAddr<uintptr_t> DragonCameraState_Vfunc_Hook(0x004F9BAF);
	// ??_7HorseCameraState@@6B@ vtbl last function + 0x5F
	RelocAddr<uintptr_t> HorseCameraState_Vfunc_Hook(0x00849E3F);

	bool Patch()
	{
		_MESSAGE("- vertical look sensitivity -");

		_MESSAGE("patching third person state...");
		{
			struct ThirdPersonStateHook_Code : Xbyak::CodeGenerator
			{
				ThirdPersonStateHook_Code(void* buf) : CodeGenerator(4096, buf)
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

			void *codeBuf = g_localTrampoline.StartAlloc();
			ThirdPersonStateHook_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(ThirdPersonState_Vfunc_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
		}
		_MESSAGE("success");

		_MESSAGE("patching dragon camera state...");
		{
			struct DragonCameraStateHook_Code : Xbyak::CodeGenerator
			{
				DragonCameraStateHook_Code(void* buf) : CodeGenerator(4096, buf)
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

			void *codeBuf = g_localTrampoline.StartAlloc();
			DragonCameraStateHook_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(DragonCameraState_Vfunc_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
		}
		_MESSAGE("success");

		_MESSAGE("patching horse camera state...");
		{
			struct HorseCameraStateHook_Code : Xbyak::CodeGenerator
			{
				HorseCameraStateHook_Code(void* buf) : CodeGenerator(4096, buf)
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

			void *codeBuf = g_localTrampoline.StartAlloc();
			HorseCameraStateHook_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(HorseCameraState_Vfunc_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
		}
		_MESSAGE("success");

		return true;
	}
}

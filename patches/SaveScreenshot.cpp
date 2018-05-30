#include "../../skse64_common/Relocation.h"
#include "../../skse64_common/SafeWrite.h"
#include "../../skse64_common/BranchTrampoline.h"
#include "../../xbyak/xbyak.h"

#include "../../skse64/GameSettings.h"

namespace SaveScreenshot
{
	uint32_t edi_saved;

	// 41 89 5d 00 40 84 ff 0f 85 + 0x8
	RelocAddr<uintptr_t> ScreenshotJnz(0x012AEDAD);
	// + 0x128 from ^
	RelocAddr<uintptr_t> RenderTargetHook_1(0x012AEED5);
	// + 0x85
	RelocAddr<uintptr_t> RenderTargetHook_2(0x012AEF5A);

	bool Patch()
	{
		_MESSAGE("- quicksave screenshot delay -");

		if (GetINISetting("bUseTAA:Display")->data.u8 >= 1 && GetINISetting("bDoDepthOfField:Imagespace")->data.u8 >= 1)
		{
			_MESSAGE("you have TAA & DOF enabled, fix uneeded");
			return true;
		}

		_MESSAGE("bypassing screenshot check");

		unsigned char nops[] = {0x90, 0x90, 0x90, 0x90, 0x90, 0x90};
		SafeWriteBuf(ScreenshotJnz.GetUIntPtr(), nops, 6);

		_MESSAGE("patching screenshot render target");
		{
			struct RenderTargetHook_1_Code : Xbyak::CodeGenerator
			{
				RenderTargetHook_1_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
				{
					// .text:00000001412AEED5                 mov     r9d, 4Bh
					// .text:00000001412AEEDB                 mov     r8d, edi
					push(rax);
					mov(rax, (uintptr_t)&edi_saved);
					mov(dword[rax], edi);
					pop(rax);
					mov(edi, 1);
					mov(r9d, 0x4B);
					mov(r8d, edi);

					jmp(ptr[rip]);
					//.text:00000001412AEEDE                 mov     rdx, [rax + 110h]
					// 12AEED5+0x9
					dq(RenderTargetHook_1.GetUIntPtr() + 0x9);
				}
			};

			void *codeBuf = g_localTrampoline.StartAlloc();
			RenderTargetHook_1_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(RenderTargetHook_1.GetUIntPtr(), uintptr_t(code.getCode()));
		}

		{
			struct RenderTargetHook_2_Code : Xbyak::CodeGenerator
			{
				RenderTargetHook_2_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
				{
					// .text:00000001412AEF5A                 mov     [rbp+218h], rax
					mov(ptr[rbp + 0x218], rax);
					push(rax);
					mov(rax, (uintptr_t)&edi_saved);
					mov(edi, dword[rax]);
					pop(rax);

					jmp(ptr[rip]);
					dq(RenderTargetHook_2.GetUIntPtr() + 0x7);
				}
			};

			void *codeBuf = g_localTrampoline.StartAlloc();
			RenderTargetHook_2_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(RenderTargetHook_2.GetUIntPtr(), uintptr_t(code.getCode()));
		}
		_MESSAGE("succces");
		return true;
	}
}
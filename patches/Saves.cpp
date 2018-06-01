#include "../../skse64_common/Relocation.h"
#include "../../skse64_common/SafeWrite.h"
#include "../../skse64_common/BranchTrampoline.h"
#include "../../xbyak/xbyak.h"

#include "../config.h"
#include "../../skse64/GameSettings.h"

// just some notes on save/load request event types
//
// D0000010   load from ingame journal menu
// 50000020   "5" unknown
// F0000040   autosave (shows "Autosave.." message and is autosave)         calls save func with (3,0,0) args
// F0000080   normal? save (shows "Autosave.." message and is normal save)  doesn't call save function?? calls a different one 
// D0000100   load last save ("Continue" on main menu)
// F0000200   quicksave  													calls save func with (4,0,0) args 
// D0000400   quickload      
// F0000800   normal? save (shows "Autosave.." message and is normal save)  doesn't call save function?? calls a different one
// 50001000   "5" unknown
// D0002000   unknown load
//
// save from ingame menu doesn't run through event handler but calls save func with (2,0,0) args

namespace Saves
{
	const uint32_t regular_save = 0xF0000080;
	const uint32_t load_last_save = 0xD0000100;

	uint32_t edi_saved;

	// 84 C0 75 26 E8 ? ? ? ? 
	RelocAddr<uintptr_t> BGSSaveLoadManager_ProcessEvents_ScreenshotCheck(0x0058A06C);

	// 41 89 5d 00 40 84 ff 0f 85 + 0x8
	RelocAddr<uintptr_t> ScreenshotJnz(0x012AEDAA);

	// + 0x128 from ^^
	RelocAddr<uintptr_t> RenderTargetHook_1(0x012AEED5);
	// + 0x85
	RelocAddr<uintptr_t> RenderTargetHook_2(0x012AEF5A);

	// QuickSaveLoadHandler::HandleEvent (vtbl 5)
	RelocAddr<uintptr_t> QuickSaveLoadHandler_HandleEvent_SaveType(0x008AAE58);
	RelocAddr<uintptr_t> QuickSaveLoadHandler_HandleEvent_LoadType(0x008AAE8B);

	RelocAddr<uintptr_t> SaveScreenshotRequested(0x02F5F968);

	RelocAddr<uintptr_t> loc_1412AF045(0x012AF045);


	bool Patch()
	{
		_MESSAGE("- save game patches -");

		if (config::regularQuicksaves)
		{
			_MESSAGE("patching quicksaves to regular saves");
			SafeWrite32(QuickSaveLoadHandler_HandleEvent_SaveType.GetUIntPtr(), regular_save);
			SafeWrite32(QuickSaveLoadHandler_HandleEvent_LoadType.GetUIntPtr(), load_last_save);
		}

		if (GetINISetting("bUseTAA:Display")->data.u8 >= 1)
		{
			_MESSAGE("you have TAA enabled, those fixes are uneeded");
			return true;
		}

		if (config::blankScreenshotsMenu)
		{
			_MESSAGE("patching in-game save delay");
			_MESSAGE("patching screenshot render (mode: temporary black journal menu)");
			{
				struct ScreenshotRender_Code : Xbyak::CodeGenerator
				{
					ScreenshotRender_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
					{
						Xbyak::Label screenRequested;

						// .text:00000001412AEDAA                 test    dil, dil
						// .text:00000001412AEDAD                 jnz     loc_1412AF045
						// .text:00000001412AEDB3                 mov     edi, 2Ah
						// .text:00000001412AEDB8                 mov     rax, [rbp + 1F0h]
						// .text:00000001412AEDBF                 cmp     byte ptr[rax + 18h], 0

						push(rax);
						mov(rax, ptr[rip + screenRequested]);
						cmp(dword[rax], 1);
						pop(rax);
						jne("ORIG");
						mov(edi, 0x2A);
						cmp(byte[rbp + 0x211], 0);
						jmp("JMP_OUT");

						L("ORIG");
						test(dil, dil);
						jnz("ORIG_JNZ");
						mov(edi, 0x2A);
						mov(rax, ptr[rbp + 0x1F0]);
						cmp(byte[rax + 0x18], 0);

						L("JMP_OUT");
						jmp(ptr[rip]);
						dq(ScreenshotJnz.GetUIntPtr() + 0x19);

						L("ORIG_JNZ");
						jmp(ptr[rip]);
						dq(loc_1412AF045.GetUIntPtr());

						L(screenRequested);
						dq(SaveScreenshotRequested.GetUIntPtr());
					}
				};

				void *codeBuf = g_localTrampoline.StartAlloc();
				ScreenshotRender_Code code(codeBuf);
				g_localTrampoline.EndAlloc(code.getCurr());

				g_branchTrampoline.Write6Branch(ScreenshotJnz.GetUIntPtr(), uintptr_t(code.getCode()));
			}
		}
		else if (config::blankScreenshotsFlicker)
		{
			_MESSAGE("patching in-game save delay");
			_MESSAGE("patching screenshot render (mode: flickering quicksaves)");
			unsigned char nops[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
			SafeWriteBuf(ScreenshotJnz.GetUIntPtr(), nops, sizeof nops);

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
		}
		else if (config::quickSaveDelay)
		{
			_MESSAGE("patching in-game save delay");
			SafeWrite8(BGSSaveLoadManager_ProcessEvents_ScreenshotCheck.GetUIntPtr(), 0xEB); // replace jnz with jmp
		}

		_MESSAGE("patching screenshot render (mode: flickering quicksaves)");


		_MESSAGE("success");
		return true;
	}
}
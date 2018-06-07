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

	// 0 = none, 1 = BGSSaveManager::ProcessEvents, 2 = open menu
	byte screenshot_requested_location = 0;

	// 84 C0 75 26 E8 ? ? ? ?  + 0x9
	RelocAddr<uintptr_t> BGSSaveLoadManager_ProcessEvents_RequestScreenshot(0x0058A073);

	RelocAddr<uintptr_t> MenuSave_RequestScreenshot(0x005B1A3A);

	// 41 89 5d 00 40 84 ff 0f 85 + 0x8
	RelocAddr<uintptr_t> ScreenshotJnz(0x012AEDAA);

	// + 0x128 from ^^
	RelocAddr<uintptr_t> RenderTargetHook_1(0x012AEED5);
	// + 0x85
	RelocAddr<uintptr_t> RenderTargetHook_2(0x012AEF5A);

	RelocAddr<uintptr_t> SaveScreenshotRequestedDword(0x02F5F968);

	RelocAddr<uintptr_t> loc_1412AF045(0x012AF045);



	// QuickSaveLoadHandler::HandleEvent (vtbl 5)
	RelocAddr<uintptr_t> QuickSaveLoadHandler_HandleEvent_SaveType(0x008AAE58);
	RelocAddr<uintptr_t> QuickSaveLoadHandler_HandleEvent_LoadType(0x008AAE8B);

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

		if (config::fixTaaSaveBugs)
		{
			// lol so we have one fix that causes flicker during quicksave and one fix that causes blank journal menus
			// so just combine both, duh
			_MESSAGE("patching in-game save delay & blank screenshot bugs");
			{
				struct IsSaveRequest_Code : Xbyak::CodeGenerator
				{
					IsSaveRequest_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
					{
						push(rax);
						// from BGSSaveLoadManager::ProcessEvent
						// this applies to all saves except saves done from the journal menu
						// since menu saves do not use the event processor; the game screenshot is actually saved
						// when you first open the menu
						mov(rax, (uintptr_t)&screenshot_requested_location);
						mov(byte[rax], 1);
						pop(rax);
						// we're replacing some nops here so we dont need to worry about original code...
						jmp(ptr[rip]);
						dq(BGSSaveLoadManager_ProcessEvents_RequestScreenshot.GetUIntPtr() + 0xD);
					}
				};

				void *codeBuf = g_localTrampoline.StartAlloc();
				IsSaveRequest_Code code(codeBuf);
				g_localTrampoline.EndAlloc(code.getCurr());

				g_branchTrampoline.Write6Branch(BGSSaveLoadManager_ProcessEvents_RequestScreenshot.GetUIntPtr(), uintptr_t(code.getCode()));

			}

			{
				struct MenuSave_Code : Xbyak::CodeGenerator
				{
					MenuSave_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
					{
						Xbyak::Label requestScreenshot;

						push(rax);
						// from open menu
						// note: this actually ends up called twice
						// after the first call the menu is open but has the regular game background
						// after the second call the game background is blurred
						mov(rax, (uintptr_t)&screenshot_requested_location);
						mov(byte[rax], 2);
						// since we overwrote the call to the request screenshot set function, just set it here
						mov(rax, ptr[rip + requestScreenshot]);
						mov(dword[rax], 1);
						pop(rax);
						jmp(ptr[rip]);
						dq(MenuSave_RequestScreenshot.GetUIntPtr() + 0x5);

						L(requestScreenshot);
						dq(SaveScreenshotRequestedDword.GetUIntPtr());
					}
				};

				void * codeBuf = g_localTrampoline.StartAlloc();
				MenuSave_Code code(codeBuf);
				g_localTrampoline.EndAlloc(code.getCurr());

				// warning: 5 byte branch instead of 6 byte branch 
				g_branchTrampoline.Write5Branch(MenuSave_RequestScreenshot.GetUIntPtr(), uintptr_t(code.getCode()));
			}

			{
				struct ScreenshotRender_Code : Xbyak::CodeGenerator
				{
					ScreenshotRender_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
					{
						// .text:00000001412AEDAA                 test    dil, dil
						// .text:00000001412AEDAD                 jnz     loc_1412AF045
						// .text:00000001412AEDB3                 mov     edi, 2Ah
						// .text:00000001412AEDB8                 mov     rax, [rbp + 1F0h]
						// .text:00000001412AEDBF                 cmp     byte ptr[rax + 18h], 0

						push(rax);
						mov(rax, (uintptr_t)&screenshot_requested_location);
						cmp(byte[rax], 2);
						je("FROM_MENU");
						cmp(byte[rax], 1);
						je("FROM_PROCESSEVENT");

						L("ORIG"); // original version of code runs if no screenshot request - i think this possibly happens when you open inventory menus etc but i didnt check
						pop(rax);
						test(dil, dil);
						jnz("ORIG_JNZ");
						L("SKIP_JNZ");
						mov(edi, 0x2A);
						mov(rax, ptr[rbp + 0x1F0]);
						cmp(byte[rax + 0x18], 0);
						jmp("JMP_OUT");

						L("FROM_MENU"); // use flicker version of fix here, all we need to do is skip jnz and rely on other patches
						pop(rax);
						jmp("SKIP_JNZ");

						L("FROM_PROCESSEVENT"); // use menu version of fix here
						mov(byte[rax], 0); // screenshot request processed disable code for future iterations
						pop(rax);
						mov(edi, 0x2A); // menu version of fix
						cmp(byte[rbp + 0x211], 0);
						jmp("JMP_OUT");

						L("JMP_OUT");
						jmp(ptr[rip]);
						dq(ScreenshotJnz.GetUIntPtr() + 0x19);

						L("ORIG_JNZ");
						jmp(ptr[rip]);
						dq(loc_1412AF045.GetUIntPtr());
					}
				};

				void *codeBuf = g_localTrampoline.StartAlloc();
				ScreenshotRender_Code code(codeBuf);
				g_localTrampoline.EndAlloc(code.getCurr());

				g_branchTrampoline.Write6Branch(ScreenshotJnz.GetUIntPtr(), uintptr_t(code.getCode()));
			}

			// flicker version of fix, checks for screenshot requested from open menu
			{
				struct RenderTargetHook_1_Code : Xbyak::CodeGenerator
				{
					RenderTargetHook_1_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
					{
						Xbyak::Label screenRequested;

						// .text:00000001412AEED5                 mov     r9d, 4Bh
						// .text:00000001412AEEDB                 mov     r8d, edi
						push(rax);
						mov(rax, (uintptr_t)&screenshot_requested_location);
						cmp(byte[rax], 2);
						jne("ORIG");
						mov(rax, (uintptr_t)&edi_saved);
						mov(dword[rax], edi);
						mov(edi, 1);

						L("ORIG");
						pop(rax);
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
						mov(rax, (uintptr_t)&screenshot_requested_location);
						cmp(byte[rax], 2);
						jne("ORIG");
						mov(byte[rax], 0);
						mov(rax, (uintptr_t)&edi_saved);
						mov(edi, dword[rax]);

						L("ORIG");
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

		_MESSAGE("success");
		return true;
	}
}
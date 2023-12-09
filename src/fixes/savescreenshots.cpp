#include "fixes.h"

namespace fixes
{
    std::uint32_t esi_saved;

    // 0 = none, 1 = BGSSaveManager::ProcessEvents, 2 = open menu
    std::uint8_t screenshot_requested_location = 0;

    REL::Relocation<std::uintptr_t> BGSSaveLoadManager_ProcessEvents_RequestScreenshot{ offsets::SaveScreenshots::BGSSaveLoadManager_ProcessEvents_RequestScreenshot, 0x1C6 };
    REL::Relocation<std::uintptr_t> MenuSave_RequestScreenshot{ offsets::SaveScreenshots::MenuSave_RequestScreenshot, 0x5D5 };
    REL::Relocation<std::uintptr_t> SaveScreenshotRequestedDword{ offsets::SaveScreenshots::g_RequestSaveScreenshot };
    REL::Relocation<std::uintptr_t> ScreenshotJnz{ offsets::SaveScreenshots::ScreenshotRenderFunction, 0x17D };
    REL::Relocation<std::uintptr_t> RenderTargetHook_1{ offsets::SaveScreenshots::ScreenshotRenderFunction, 0x294 };
    REL::Relocation<std::uintptr_t> RenderTargetHook_2{ offsets::SaveScreenshots::ScreenshotRenderFunction, 0x307 };
    REL::Relocation<std::uintptr_t> ScreenshotRenderOrigJnz{ offsets::SaveScreenshots::ScreenshotRenderFunction, 0x3B1 };

    bool PatchSaveScreenshots()
    {
        logger::trace("- save game screenshot fix -"sv);

        if (RE::GetINISetting("bUseTAA:Display")->GetBool())
        {
            logger::trace("you have TAA enabled, those fixes are uneeded"sv);
            return true;
        }

        if (*config::fixSaveScreenshots)
        {
            // lol so we have one fix that causes flicker during quicksave and one fix that causes blank journal menus
            // so just combine both, duh
            logger::trace("patching in-game save delay & blank screenshot bugs"sv);

            // with DoF enabled just use the "flicker" fix even for ingame requests
            if (RE::GetINISetting("bDoDepthOfField:Imagespace")->GetBool())
            {
                struct IsSaveRequest_Code : Xbyak::CodeGenerator
                {
                    IsSaveRequest_Code()
                    {
                        push(rax);
                        // from BGSSaveLoadManager::ProcessEvent
                        // this applies to all saves except saves done from the journal menu
                        // since menu saves do not use the event processor; the game screenshot is actually saved
                        // when you first open the menu
                        mov(rax, (std::uintptr_t)&screenshot_requested_location);
                        mov(byte[rax], 2);
                        pop(rax);
                        // we're replacing some nops here so we dont need to worry about original code...
                        jmp(ptr[rip]);
                        dq(BGSSaveLoadManager_ProcessEvents_RequestScreenshot.address() + 0xA);
                    }
                };

                IsSaveRequest_Code code;
                code.ready();

                auto& trampoline = SKSE::GetTrampoline();
                trampoline.write_branch<6>(
                    BGSSaveLoadManager_ProcessEvents_RequestScreenshot.address(),
                    trampoline.allocate(code));
            }
            // use menu fix for DoF+TAA Disabled ingame requests
            else
            {
                struct IsSaveRequest_Code : Xbyak::CodeGenerator
                {
                    IsSaveRequest_Code()
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
                        dq(BGSSaveLoadManager_ProcessEvents_RequestScreenshot.address() + 0xA);
                    }
                };

                IsSaveRequest_Code code;
                code.ready();

                auto& trampoline = SKSE::GetTrampoline();
                trampoline.write_branch<6>(
                    BGSSaveLoadManager_ProcessEvents_RequestScreenshot.address(),
                    trampoline.allocate(code));
            }

            // flicker fix for open menu screenshot requests
            {
                struct MenuSave_Code : Xbyak::CodeGenerator
                {
                    MenuSave_Code()
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
                        dq(MenuSave_RequestScreenshot.address() + 0x5);

                        L(requestScreenshot);
                        dq(SaveScreenshotRequestedDword.address());
                    }
                };

                MenuSave_Code code;
                code.ready();

                auto& trampoline = SKSE::GetTrampoline();
                // warning: 5 byte branch instead of 6 byte branch
                trampoline.write_branch<5>(
                    MenuSave_RequestScreenshot.address(),
                    trampoline.allocate(code));
            }

            {
                struct ScreenshotRender_Code : Xbyak::CodeGenerator
                {
                    ScreenshotRender_Code()
                    {
                        /*
                        * .text:00000001413BD81D                 test    dil, dil
                        * .text:00000001413BD820                 jnz     loc_1413BDA51
                        * .text:00000001413BD826                 mov     rcx, [rbx+1F0h]
                        * .text:00000001413BD82D                 mov     esi, 2Ah ; '*'
                        * .text:00000001413BD832                 cmp     [rcx+18h], dil
                        */

                        push(rax);
                        mov(rax, (uintptr_t)&screenshot_requested_location);
                        cmp(byte[rax], 2);
                        je("FROM_MENU");
                        cmp(byte[rax], 1);
                        je("FROM_PROCESSEVENT");

                        L("ORIG");  // original version of code runs if no screenshot request - i think this possibly happens when you open inventory menus etc but i didnt check
                        pop(rax);
                        test(dil, dil);
                        jnz("ORIG_JNZ");
                        L("SKIP_JNZ");
                        mov(rcx, ptr[rbx + 0x1F0]);
                        mov(esi, 0x2A);
                        cmp(byte[rcx + 0x18], dil);
                        jmp("JMP_OUT");

                        L("FROM_MENU");  // use flicker version of fix here, all we need to do is skip jnz and rely on other patches
                        pop(rax);
                        jmp("SKIP_JNZ");

                        L("FROM_PROCESSEVENT");  // use menu version of fix here
                        mov(byte[rax], 0);       // screenshot request processed disable code for future iterations
                        pop(rax);
                        mov(rcx, ptr[rbx + 0x1F0]);
                        mov(esi, 0x2A);  // menu version of fix
                        cmp(byte[rbx + 0x211], 0);
                        jmp("JMP_OUT");

                        L("JMP_OUT");
                        jmp(ptr[rip]);
                        dq(ScreenshotJnz.address() + 0x19);

                        L("ORIG_JNZ");
                        jmp(ptr[rip]);
                        dq(ScreenshotRenderOrigJnz.address());
                    }
                };

                ScreenshotRender_Code code;
                code.ready();

                auto& trampoline = SKSE::GetTrampoline();
                trampoline.write_branch<6>(
                    ScreenshotJnz.address(),
                    trampoline.allocate(code));
            }

            // flicker version of fix, checks for screenshot requested from open menu
            {
                struct RenderTargetHook_1_Code : Xbyak::CodeGenerator
                {
                    RenderTargetHook_1_Code()
                    {
                        Xbyak::Label screenRequested;

                        // .text:00000001413BD934                 mov     r8d, esi
                        // .text:00000001413BD937                 mov     rcx, rbx
                        push(rax);
                        mov(rax, (uintptr_t)&screenshot_requested_location);
                        cmp(byte[rax], 2);
                        jne("ORIG");
                        mov(rax, (uintptr_t)&esi_saved);
                        mov(dword[rax], esi);
                        mov(esi, 1);

                        L("ORIG");
                        pop(rax);
                        mov(r8d, esi);
                        mov(rcx, rbx);

                        jmp(ptr[rip]);
                        // .text:00000001413BD93A                 lea     edx, [r9-29h]
                        dq(RenderTargetHook_1.address() + 0x6);
                    }
                };

                RenderTargetHook_1_Code code;
                code.ready();

                auto& trampoline = SKSE::GetTrampoline();
                trampoline.write_branch<6>(
                    RenderTargetHook_1.address(),
                    trampoline.allocate(code));
            }

            {
                struct RenderTargetHook_2_Code : Xbyak::CodeGenerator
                {
                    RenderTargetHook_2_Code()
                    {
                        // .text:00000001413BD9A7                 mov     [rbx+218h], rax
                        mov(ptr[rbx + 0x218], rax);
                        push(rax);
                        mov(rax, (uintptr_t)&screenshot_requested_location);
                        cmp(byte[rax], 2);
                        jne("ORIG");
                        mov(byte[rax], 0);
                        mov(rax, (uintptr_t)&esi_saved);
                        mov(esi, dword[rax]);

                        L("ORIG");
                        pop(rax);
                        jmp(ptr[rip]);
                        dq(RenderTargetHook_2.address() + 0x7);
                    }
                };

                RenderTargetHook_2_Code code;
                code.ready();

                auto& trampoline = SKSE::GetTrampoline();
                trampoline.write_branch<6>(
                    RenderTargetHook_2.address(),
                    trampoline.allocate(code));
            }
        }

        logger::trace("success"sv);
        return true;
    }
}

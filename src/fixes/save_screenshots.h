#pragma once

namespace Fixes::SaveScreenshots
{
    namespace detail
    {
        inline std::uint32_t saved_register;

        // 0 = none, 1 = BGSSaveManager::ProcessEvents, 2 = open menu
        inline std::uint8_t screenshot_requested_location = 0;

        inline void Install()
        {
            REL::Relocation BGSSaveLoadManager_ProcessEvents_RequestScreenshot{ RELOCATION_ID(34682, 35772), VAR_NUM(0x163, 0x1C6) };
            REL::Relocation MenuSave_RequestScreenshot{ RELOCATION_ID(35556, 36555), VAR_NUM(0x56A, 0x5D5) };
            REL::Relocation SaveScreenshotRequestedDword{ RELOCATION_ID(517224, 403755) };
            REL::Relocation ScreenshotJnz{ RELOCATION_ID(99023, 105674), VAR_NUM(0x23A, 0x17D) };
            REL::Relocation RenderTargetHook_1{ RELOCATION_ID(99023, 105674), VAR_NUM(0x365, 0x294) };
            REL::Relocation RenderTargetHook_2{ RELOCATION_ID(99023, 105674), VAR_NUM(0x3EA, 0x307) };
            REL::Relocation ScreenshotRenderOrigJnz{ RELOCATION_ID(99023, 105674), VAR_NUM(0x4D5, 0x3B1) };

            if (RE::GetINISetting("bUseTAA:Display")->GetBool()) {
                return;
            }

            auto& trampoline = SKSE::GetTrampoline();

            // have one fix that causes flicker during quicksave and one fix that causes blank journal menus
            // so just combine both
            // with DoF enabled just use the "flicker" fix even for ingame requests
            if (RE::GetINISetting("bDoDepthOfField:Imagespace")->GetBool()) {
                struct IsSaveRequest_Code : Xbyak::CodeGenerator
                {
                    explicit IsSaveRequest_Code(std::uintptr_t a_target)
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
                        dq(a_target + VAR_NUM(0xD, 0xA));
                    }
                };

                IsSaveRequest_Code code(BGSSaveLoadManager_ProcessEvents_RequestScreenshot.address());
                code.ready();

                BGSSaveLoadManager_ProcessEvents_RequestScreenshot.write_branch<6>(trampoline.allocate(code));
            }
            // use menu fix for DoF+TAA Disabled ingame requests
            else {
                struct IsSaveRequest_Code : Xbyak::CodeGenerator
                {
                    explicit IsSaveRequest_Code(std::uintptr_t a_target)
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
                        dq(a_target + VAR_NUM(0xD, 0xA));
                    }
                };

                IsSaveRequest_Code code(BGSSaveLoadManager_ProcessEvents_RequestScreenshot.address());
                code.ready();

                BGSSaveLoadManager_ProcessEvents_RequestScreenshot.write_branch<6>(trampoline.allocate(code));
            }

            // flicker fix for open menu screenshot requests
            {
                struct MenuSave_Code : Xbyak::CodeGenerator
                {
                    MenuSave_Code(std::uintptr_t a_target, std::uintptr_t a_constant)
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
                        dq(a_target + 0x5);

                        L(requestScreenshot);
                        dq(a_constant);
                    }
                };

                MenuSave_Code code(MenuSave_RequestScreenshot.address(), SaveScreenshotRequestedDword.address());
                code.ready();

                // warning: 5 byte branch instead of 6 byte branch
                MenuSave_RequestScreenshot.write_branch<5>(trampoline.allocate(code));
            }

            {
                struct ScreenshotRender_Code : Xbyak::CodeGenerator
                {
                    ScreenshotRender_Code(std::uintptr_t a_target, std::uintptr_t a_altJmp)
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
#ifdef SKYRIM_AE
                        mov(rcx, ptr[rbx + 0x1F0]);
                        mov(esi, 0x2A);
                        cmp(byte[rcx + 0x18], dil);
#else
                        mov(edi, 0x2A);
                        mov(rax, ptr[rbp + 0x1F0]);
                        cmp(byte[rax + 0x18], 0);
#endif
                        jmp("JMP_OUT");

                        L("FROM_MENU");  // use flicker version of fix here, all we need to do is skip jnz and rely on other patches
                        pop(rax);
                        jmp("SKIP_JNZ");

                        L("FROM_PROCESSEVENT");  // use menu version of fix here
                        mov(byte[rax], 0);       // screenshot request processed disable code for future iterations
                        pop(rax);
#ifdef SKYRIM_AE
                        mov(rcx, ptr[rbx + 0x1F0]);
                        mov(esi, 0x2A);  // menu version of fix
                        cmp(byte[rbx + 0x211], 0);
#else
                        mov(edi, 0x2A);  // menu version of fix
                        cmp(byte[rbp + 0x211], 0);
#endif
                        jmp("JMP_OUT");

                        L("JMP_OUT");
                        jmp(ptr[rip]);
                        dq(a_target + 0x19);

                        L("ORIG_JNZ");
                        jmp(ptr[rip]);
                        dq(a_altJmp);
                    }
                };

                ScreenshotRender_Code code(ScreenshotJnz.address(), ScreenshotRenderOrigJnz.address());
                code.ready();

                ScreenshotJnz.write_branch<6>(trampoline.allocate(code));
            }

            // flicker version of fix, checks for screenshot requested from open menu
            {
                struct RenderTargetHook_1_Code : Xbyak::CodeGenerator
                {
                    explicit RenderTargetHook_1_Code(std::uintptr_t a_target)
                    {
                        Xbyak::Label screenRequested;

                        // .text:00000001413BD934                 mov     r8d, esi
                        // .text:00000001413BD937                 mov     rcx, rbx
                        push(rax);
                        mov(rax, (uintptr_t)&screenshot_requested_location);
                        cmp(byte[rax], 2);
                        jne("ORIG");
                        mov(rax, (uintptr_t)&saved_register);
#ifdef SKYRIM_AE
                        mov(dword[rax], esi);
                        mov(esi, 1);
#else
                        mov(dword[rax], edi);
                        mov(edi, 1);
#endif

                        L("ORIG");
                        pop(rax);
#ifdef SKYRIM_AE
                        mov(r8d, esi);
                        mov(rcx, rbx);
#else
                        mov(r9d, 0x4B);
                        mov(r8d, edi);
#endif

                        jmp(ptr[rip]);
                        // .text:00000001413BD93A                 lea     edx, [r9-29h]
                        dq(a_target + VAR_NUM(0x9, 0x6));
                    }
                };

                RenderTargetHook_1_Code code(RenderTargetHook_1.address());
                code.ready();

                RenderTargetHook_1.write_branch<6>(trampoline.allocate(code));
            }

            {
                struct RenderTargetHook_2_Code : Xbyak::CodeGenerator
                {
                    explicit RenderTargetHook_2_Code(std::uintptr_t a_target)
                    {
                        // .text:00000001413BD9A7                 mov     [rbx+218h], rax
#ifdef SKYRIM_AE
                        mov(ptr[rbx + 0x218], rax);
#else
                        mov(ptr[rbp + 0x218], rax);
#endif
                        push(rax);
                        mov(rax, (uintptr_t)&screenshot_requested_location);
                        cmp(byte[rax], 2);
                        jne("ORIG");
                        mov(byte[rax], 0);
                        mov(rax, (uintptr_t)&saved_register);
#ifdef SKYRIM_AE
                        mov(esi, dword[rax]);
#else
                        mov(edi, dword[rax]);
#endif

                        L("ORIG");
                        pop(rax);
                        jmp(ptr[rip]);
                        dq(a_target + 0x7);
                    }
                };

                RenderTargetHook_2_Code code(RenderTargetHook_2.address());
                code.ready();

                RenderTargetHook_2.write_branch<6>(trampoline.allocate(code));
            }
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed save screenshots fix"sv);
    }
}
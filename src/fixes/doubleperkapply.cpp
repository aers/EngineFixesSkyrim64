#include "fixes.h"

namespace fixes
{
    std::uint32_t next_formid;

    typedef void (*_QueueApplyPerk)(RE::TaskQueueInterface* thisPtr, RE::Actor* actor, RE::BGSPerk* perk, std::int8_t oldRank, std::int8_t newRank);
    REL::Relocation<_QueueApplyPerk> QueueApplyPerk{ offsets::DoublePerkApply::QueueApplyPerk };
    typedef void (*_HandleAddRf)(std::int64_t apm);
    REL::Relocation<_HandleAddRf> HandleAddRf{ offsets::DoublePerkApply::Handle_Add_Rf };
    REL::Relocation<std::uintptr_t> SwitchFunctionMovzx{ offsets::DoublePerkApply::BSTaskPool_HandleTask_Movzx, 0x2164 };
    REL::Relocation<std::uintptr_t> UnknownAddFuncMovzx1{ offsets::DoublePerkApply::Unknown_Add_Function, 0x1A };
    REL::Relocation<std::uintptr_t> UnknownAddFuncMovzx2{ offsets::DoublePerkApply::Unknown_Add_Function, 0x46 };
    REL::Relocation<std::uintptr_t> NextFormIdGetHook{ offsets::DoublePerkApply::Next_Formid_Get_Hook, 0x1B };
    REL::Relocation<std::uintptr_t> DoHandleHook{ offsets::DoublePerkApply::Do_Handle_Hook, 0x11 };
    REL::Relocation<std::uintptr_t> DoAddHook{ offsets::DoublePerkApply::Do_Add_Hook, 0x11 };

    void do_add(RE::Actor* actorPtr, RE::BGSPerk* perkPtr, std::int8_t newRank)
    {
        std::int8_t oldRank = 0;
        const auto formid = actorPtr->GetFormID();

        if (formid == next_formid)
        {
            //_DMESSAGE("perk loop in formid %08X", formid);
            next_formid = 0;
            if (formid != 0x14)  // player formid = 0x14
                oldRank |= 0x100;
        }

        QueueApplyPerk(RE::TaskQueueInterface::GetSingleton(), actorPtr, perkPtr, oldRank, newRank);
    }

    void do_handle(std::int64_t actorPtr, std::uint32_t val)
    {
        bool shouldClear = (val & 0x100) != 0;

        if (shouldClear)
        {
            std::int64_t apm = *((std::int64_t*)(actorPtr + 0xF0));  // actorprocessmanager 0xF0 in SSE Actor
            if (apm != 0)
                HandleAddRf(apm);
        }
    }

    bool PatchDoublePerkApply()
    {
        logger::trace("- double perk apply -"sv);

        logger::trace("patching val movzx"sv);
        // .text:00000001405C8FDE                 movzx   r8d, byte ptr [rdi+18h]
        // 44 0F B6 47 18
        // ->
        // mov r8d, dword ptr [rdi+18h]
        // 44 8b 47 18 90
        std::uint8_t first_movzx_patch[] = { 0x44, 0x8b, 0x47, 0x18, 0x90 };
        REL::safe_write(SwitchFunctionMovzx.address(), first_movzx_patch, 5);

        // .text:00000001405C6C6A                 movzx   edi, r9b
        // 41 0F B6 F9
        // ->
        // mov edi, r9d
        // 44 89 CF 90
        std::uint8_t second_movzx_patch[] = { 0x44, 0x89, 0xCF, 0x90 };
        REL::safe_write(UnknownAddFuncMovzx1.address(), second_movzx_patch, 4);

        // .text:00000001405C6C96                 movzx   eax, dil
        // 40 0F B6 C7
        // ->
        // mov eax, edi
        // 89 F8 90 90
        std::uint8_t third_movzx_patch[] = { 0x89, 0xF8, 0x90, 0x90 };
        REL::safe_write(UnknownAddFuncMovzx2.address(), third_movzx_patch, 4);

        logger::trace("hooking for next form ID"sv);
        {
            struct GetNextFormId_Code : Xbyak::CodeGenerator
            {
                GetNextFormId_Code()
                {
                    Xbyak::Label retnLabel;

                    // enter 68249B
                    //.text:000000014068249B                 mov[rsp + 38h + var_10], rdx
                    mov(ptr[rsp + 0x28], rdx);
                    // store formid
                    push(rax);
                    mov(rdx, qword[rdx + 0x14]);
                    mov(rax, (uintptr_t)&next_formid);
                    mov(dword[rax], edx);
                    pop(rax);
                    //.text:00000001406824A0                 lea     rdx, [rsp + 38h + var_18]
                    lea(rdx, ptr[rsp + 0x20]);
                    // exit 6824A5
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(NextFormIdGetHook.address() + 0xA);
                }
            };

            GetNextFormId_Code code;
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                NextFormIdGetHook.address(),
                trampoline.allocate(code));
        }

        logger::trace("hooking handle function"sv);
        {
            struct DoHandleHook_Code : Xbyak::CodeGenerator
            {
                DoHandleHook_Code(std::uintptr_t doHandleAddr)
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label funcLabel;

                    // enter 34EAD1
                    // .text:000000014034EAD1                 movzx   ebp, r9b
                    movzx(ebp, r9b);
                    // .text:000000014034EAD5                 movzx   r14d, r8b
                    mov(r14d, r8d); // preserve val
                    // .text:000000014034EAD9                 mov     rdi, rdx
                    mov(rdi, rdx);
                    // .text:000000014034EADC                 mov     r15, rcx
                    mov(r15, rcx);

                    // call do_handle
                    push(rcx); // preserve rcx
                    mov(edx, r14d);  // val
                    mov(rcx, rdi);   // actorPtr
                    sub(rsp, 0x20);  // parameter stack space
                                     //void do_handle(int64_t actorPtr, uint32_t val)
                    call(ptr[rip + funcLabel]);
                    add(rsp, 0x20);
                    pop(rcx);

                    // exit 34EADF
                    jmp(ptr[rip + retnLabel]);

                    L(funcLabel);
                    dq(doHandleAddr);

                    L(retnLabel);
                    dq(DoHandleHook.address() + 0xE);
                }
            };

            DoHandleHook_Code code(reinterpret_cast<std::uintptr_t>(do_handle));
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                DoHandleHook.address(),
                trampoline.allocate(code));
        }

        logger::trace("hooking add function");
        {
            struct DoAddHook_Code : Xbyak::CodeGenerator
            {
                DoAddHook_Code(std::uintptr_t doAddAddr)
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label funcLabel;

                    // .text:0000000140338CB9                 mov     r8, [rdx]			             perk ptr
                    // enter 338CC1
                    // .text:0000000140338CC1                 movzx   eax, byte ptr[rdx + 8]		 do_add unk1
                    // .text:0000000140338CC5                 xor     r9d, r9d                       val = 0
                    // .text:0000000140338CC8                 mov     rdx, [rcx + 8]                 actor ptr
                    // .text:0000000140338CCC                 mov     rcx, cs:qword_142F5F978        taskpool
                    // .text:0000000140338CD3                 mov[rsp + 38h + var_18], al            do_add unk1

                    movzx(eax, byte[rdx + 0x8]);
                    mov(rcx, ptr[rcx + 0x8]);  // actorPtr
                    mov(rdx, r8);              //perkPtr
                    mov(r8d, eax);             //unk1

                    // void do_add(int64_t actorPtr, int64_t perkPtr, int32_t unk1)
                    call(ptr[rip + funcLabel]);

                    // exit 338CDC
                    jmp(ptr[rip + retnLabel]);

                    L(funcLabel);
                    dq(doAddAddr);

                    L(retnLabel);
                    dq(DoAddHook.address() + 0x1B);
                }
            };

            DoAddHook_Code code(reinterpret_cast<std::uintptr_t>(do_add));
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            trampoline.write_branch<6>(
                DoAddHook.address(),
                trampoline.allocate(code));
        }

        logger::trace("success");

        return true;
    }
}

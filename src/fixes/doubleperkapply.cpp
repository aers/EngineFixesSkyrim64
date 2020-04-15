#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/SafeWrite.h"
#include "SKSE/Trampoline.h"

#include "fixes.h"

namespace fixes
{
    uint32_t next_formid;

    typedef void (*_QueueApplyPerk)(RE::TaskQueueInterface* thisPtr, RE::Actor* actor, RE::BGSPerk* perk, std::int8_t oldRank, std::int8_t newRank);
    REL::Offset<_QueueApplyPerk> QueueApplyPerk(QueueApplyPerk_offset);
    typedef void (*_HandleAddRf)(int64_t apm);
    REL::Offset<_HandleAddRf> HandleAddRf(Handle_Add_Rf_offset);
    REL::Offset<std::uintptr_t> SwitchFunctionMovzx(Switch_Function_movzx_offset, 0x1C4E);
    REL::Offset<std::uintptr_t> UnknownAddFuncMovzx1(Unknown_Add_Function_movzx_offset, 0x1A);
    REL::Offset<std::uintptr_t> UnknownAddFuncMovzx2(Unknown_Add_Function_movzx2_offset, 0x46);
    REL::Offset<std::uintptr_t> NextFormIdGetHook(Next_Formid_Get_Hook_offset, 0x1B);
    REL::Offset<std::uintptr_t> DoHandleHook(Do_Handle_Hook_offset, 0x1B);
    REL::Offset<std::uintptr_t> DoAddHook(Do_Add_Hook_offset, 0x11);

    void do_add(RE::Actor* actorPtr, RE::BGSPerk* perkPtr, std::int8_t newRank)
    {
        std::int8_t oldRank = 0;

        auto formid = actorPtr->GetFormID();

        if (formid == next_formid)
        {
            //_DMESSAGE("perk loop in formid %08X", formid);
            next_formid = 0;
            if (formid != 0x14)  // player formid = 0x14
                oldRank |= 0x100;
        }

        QueueApplyPerk(RE::TaskQueueInterface::GetSingleton(), actorPtr, perkPtr, oldRank, newRank);
    }

    void do_handle(int64_t actorPtr, uint32_t val)
    {
        bool shouldClear = (val & 0x100) != 0;

        if (shouldClear)
        {
            int64_t apm = *((int64_t*)(actorPtr + 0xF0));  // actorprocessmanager 0xF0 in SSE Actor
            if (apm != 0)
                HandleAddRf(apm);
        }
    }

    bool PatchDoublePerkApply()
    {
        _VMESSAGE("- double perk apply -");

        _VMESSAGE("patching val movzx");
        // .text:00000001405C8FDE                 movzx   r8d, byte ptr [rdi+18h]
        // 44 0F B6 47 18
        // ->
        // mov r8d, dword ptr [rdi+18h]
        // 44 8b 47 18 90
        unsigned char first_movzx_patch[] = { 0x44, 0x8b, 0x47, 0x18, 0x90 };
        SKSE::SafeWriteBuf(SwitchFunctionMovzx.GetAddress(), first_movzx_patch, 5);

        // .text:00000001405C6C6A                 movzx   edi, r9b
        // 41 0F B6 F9
        // ->
        // mov edi, r9d
        // 44 89 CF 90
        unsigned char second_movzx_patch[] = { 0x44, 0x89, 0xCF, 0x90 };
        SKSE::SafeWriteBuf(UnknownAddFuncMovzx1.GetAddress(), second_movzx_patch, 4);

        // .text:00000001405C6C96                 movzx   eax, dil
        // 40 0F B6 C7
        // ->
        // mov eax, edi
        // 89 F8 90 90
        unsigned char third_movzx_patch[] = { 0x89, 0xF8, 0x90, 0x90 };
        SKSE::SafeWriteBuf(UnknownAddFuncMovzx2.GetAddress(), third_movzx_patch, 4);

        _VMESSAGE("hooking for next form ID");
        {
            struct GetNextFormId_Code : SKSE::CodeGenerator
            {
                GetNextFormId_Code() : SKSE::CodeGenerator()
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
                    dq(NextFormIdGetHook.GetAddress() + 0xA);
                }
            };

            GetNextFormId_Code code;
            code.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(NextFormIdGetHook.GetAddress(), uintptr_t(code.getCode()));
        }

        _VMESSAGE("hooking handle function");
        {
            struct DoHandleHook_Code : SKSE::CodeGenerator
            {
                DoHandleHook_Code(std::uintptr_t doHandleAddr) : SKSE::CodeGenerator()
                {
                    Xbyak::Label retnLabel;
                    Xbyak::Label funcLabel;

                    // enter 33891B
                    // .text:000000014033891B                 add     rcx, 60h
                    add(rcx, 0x60);
                    // .text:000000014033891F                 movzx   ebp, r9b
                    movzx(ebp, r9b);
                    // .text:0000000140338923                 movzx   r14d, r8b
                    mov(r14d, r8d);  // preserve val

                    // call do_handle

                    push(rdx);  // save rdx+rcx
                    push(rcx);
                    mov(edx, r14d);  // val
                    mov(rcx, rsi);   // actorPtr
                    sub(rsp, 0x20);  // parameter stack space
                                     //void do_handle(int64_t actorPtr, uint32_t val)
                    call(ptr[rip + funcLabel]);
                    add(rsp, 0x20);
                    pop(rcx);
                    pop(rdx);

                    // exit 338927
                    jmp(ptr[rip + retnLabel]);

                    L(funcLabel);
                    dq(doHandleAddr);

                    L(retnLabel);
                    dq(DoHandleHook.GetAddress() + 0x8);
                }
            };

            DoHandleHook_Code code(reinterpret_cast<std::uintptr_t>(do_handle));
            code.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(DoHandleHook.GetAddress(), uintptr_t(code.getCode()));
        }

        _VMESSAGE("hooking add function");
        {
            struct DoAddHook_Code : SKSE::CodeGenerator
            {
                DoAddHook_Code(std::uintptr_t doAddAddr) : SKSE::CodeGenerator()
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
                    dq(DoAddHook.GetAddress() + 0x1B);
                }
            };

            DoAddHook_Code code(reinterpret_cast<std::uintptr_t>(do_add));
            code.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(DoAddHook.GetAddress(), uintptr_t(code.getCode()));
        }

        _VMESSAGE("success");

        return true;
    }
}

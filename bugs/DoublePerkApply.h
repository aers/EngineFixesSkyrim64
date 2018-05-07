#pragma once
#include "../skse64_common/Relocation.h"

bool PatchDoublePerkApply();

// patch offsets 
// SSE 1.5.39
#define off_UNKNOWN_ADD_FUNCTION 0x005C6C50
#define off_TASKPOOL 0x02F5F978

#define off_HANDLEADDRF 0x006824D0

#define off_SWITCHMOVZX 0x005C8FDE
#define off_ADDFUNCMOVZX1 0x005C6C6A
#define off_ADDFUNCMOVZX2 0x005C6C96

#define off_NEXTFORMIDGETHOOK 0x0068249B
#define off_DOHANDLEHOOK 0x0033891B
#define off_DOADDHOOK 0x00338CC1

// patch addresses
typedef void(*_UnknownAddFunc)(int64_t taskPool, int64_t actorPtr, int64_t perkPtr, uint32_t val, int32_t unk1);
extern RelocAddr<_UnknownAddFunc> UnknownAddFunc;
extern RelocAddr<uintptr_t *> TaskPool;

typedef void(*_HandleAddRf)(int64_t apm);
extern RelocAddr<_HandleAddRf> HandleAddRf;

// .text:00000001405C8FDE                 movzx   r8d, byte ptr [rdi+18h]
extern RelocAddr<uintptr_t> SwitchFunctionMovzx;
// .text:00000001405C6C6A                 movzx   edi, r9b
extern RelocAddr<uintptr_t> UnknownAddFuncMovzx1;
// .text:00000001405C6C96                 movzx   eax, dil
extern RelocAddr<uintptr_t> UnknownAddFuncMovzx2;
// next form id get
// .text:000000014068249B                 mov     [rsp+38h+var_10], rdx
// .text:00000001406824A0                 lea     rdx, [rsp + 38h + var_18]
extern RelocAddr<uintptr_t> NextFormIdGetHook;
// do_handle hook
// .text:000000014033891B                 add     rcx, 60h
// .text:000000014033891F                 movzx   ebp, r9b
// .text:0000000140338923                 movzx   r14d, r8b
// length 0x8
extern RelocAddr<uintptr_t> DoHandleHook;
// do_add hook
// .text:0000000140338CC1                 movzx   eax, byte ptr [rdx+8]
// .text:0000000140338CC5                 xor     r9d, r9d
// .text:0000000140338CC8                 mov     rdx, [rcx + 8]
// .text:0000000140338CCC                 mov     rcx, cs:qword_142F5F978
// .text:0000000140338CD3                 mov[rsp + 38h + var_18], al
// .text:0000000140338CD7                 call    sub_1405C6C50
// .text:0000000140338CDC loc_140338CDC:    
// length 0x1B
extern RelocAddr<uintptr_t> DoAddHook;
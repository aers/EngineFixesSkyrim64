#include "tbb/concurrent_hash_map.h"

#include "RE/BSTHashMap.h"
#include "RE/TESForm.h"

#include "skse64/GameForms.h"

#include "patches.h"


namespace patches
{
    // treelodreferencecaching.cpp
    void InvalidateCachedForm(uint32_t formId);

    tbb::concurrent_hash_map<uint32_t, RE::TESForm *> globalFormCacheMap[256];

    RelocPtr<BSReadWriteLock> GlobalFormTableLock(GlobalFormTableLock_offset);
    RelocPtr<RE::BSTHashMap<UInt32, RE::TESForm *> *> GlobalFormTable(GlobalFormTable_offset);

    typedef void(*UnknownFormFunction0_)(__int64 form, bool a2);
    RelocAddr<UnknownFormFunction0_> origFunc0HookAddr(UnkFormFunc1_offset);
    UnknownFormFunction0_ origFunc0;

     typedef __int64(*UnknownFormFunction1_)(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 *a5);
    RelocAddr <UnknownFormFunction1_> origFunc1HookAddr(UnkFormFunc2_offset);
    UnknownFormFunction1_ origFunc1;

       typedef __int64(*UnknownFormFunction2_)(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 **a5);
    RelocAddr <UnknownFormFunction2_> origFunc2HookAddr(UnkFormFunc3_offset);
    UnknownFormFunction2_ origFunc2;

       typedef __int64(*UnknownFormFunction3_)(__int64 a1, __int64 a2, int a3, __int64 a4);
    RelocAddr <UnknownFormFunction3_> origFunc3HookAddr(UnkFormFunc4_offset);
    UnknownFormFunction3_ origFunc3;

    void UpdateFormCache(uint32_t FormId, RE::TESForm *Value, bool Invalidate)
    {
        const unsigned char masterId = (FormId & 0xFF000000) >> 24;
        const unsigned int baseId = (FormId & 0x00FFFFFF);

        if (Invalidate)
            globalFormCacheMap[masterId].erase(baseId);
        else
            globalFormCacheMap[masterId].insert(std::make_pair(baseId, Value));

        InvalidateCachedForm(FormId);
    }

    RE::TESForm * hk_GetFormByID(unsigned int FormId)
    {
        RE::TESForm *formPointer = nullptr;

        const unsigned char masterId = (FormId & 0xFF000000) >> 24;
        const unsigned int baseId = (FormId & 0x00FFFFFF);

        {
            tbb::concurrent_hash_map<uint32_t, RE::TESForm *>::accessor accessor;

            if (globalFormCacheMap[masterId].find(accessor, baseId))
            {
                formPointer = accessor->second;
                return formPointer;
            }
        }

        // Try to use Bethesda's scatter table which is considerably slower
        CALL_MEMBER_FN(GlobalFormTableLock, LockForRead)();

        if (*GlobalFormTable)
        {
            auto iter = (*GlobalFormTable)->find(FormId);
            formPointer = (iter != (*GlobalFormTable)->end()) ? iter->GetValue() : nullptr;
        }

        CALL_MEMBER_FN(GlobalFormTableLock, UnlockRead)();

        if (formPointer)
            UpdateFormCache(FormId, formPointer, false);

        return formPointer;
    }

    __int64 UnknownFormFunction3(__int64 a1, __int64 a2, int a3, __int64 a4)
    {
        UpdateFormCache(*(uint32_t *)a4, nullptr, true);

        return origFunc3(a1, a2, a3, a4);
    }

    __int64 UnknownFormFunction2(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 **a5)
    {
        UpdateFormCache(*formId, nullptr, true);

        return origFunc2(a1, a2, a3, formId, a5);
    }

    __int64 UnknownFormFunction1(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 *a5)
    {
        UpdateFormCache(*formId, nullptr, true);

        return origFunc1(a1, a2, a3, formId, a5);
    }

    void UnknownFormFunction0(__int64 form, bool a2)
    {
        UpdateFormCache(*(uint32_t *)(form + 0x14), nullptr, true);

        origFunc0(form, a2);
    }


    bool PatchFormCaching()
    {
        _VMESSAGE("- form caching -");

        if (*(uint32_t *)LookupFormByID.GetUIntPtr() != 0x83485740)
        {
            _VMESSAGE("sse fixes is installed and enabled. aborting form cache patch.");
            config::patchFormCaching = false;

            return false;
        }

        _VMESSAGE("detouring GetFormById");
        g_branchTrampoline.Write6Branch(LookupFormByID.GetUIntPtr(), GetFnAddr(hk_GetFormByID));
        _VMESSAGE("done");

        // TODO: write a generic detour instead
        _VMESSAGE("detouring global form table write functions");
        {
            struct UnknownFormFunction0_Code : Xbyak::CodeGenerator
            {
                UnknownFormFunction0_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
                {
                    Xbyak::Label retnLabel;

                    // 194970
                    mov(r11, rsp);
                    push(rbp);
                    push(rsi);
                    push(rdi);
                    push(r12);
                    // 194978

                    // exit 
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc0HookAddr.GetUIntPtr() + 0x8);
                }
            };

            void *codeBuf = g_localTrampoline.StartAlloc();
            UnknownFormFunction0_Code code(codeBuf);
            g_localTrampoline.EndAlloc(code.getCurr());

            origFunc0 = UnknownFormFunction0_(code.getCode());
            g_branchTrampoline.Write6Branch(origFunc0HookAddr.GetUIntPtr(), GetFnAddr(UnknownFormFunction0));
        }

        {
            struct UnknownFormFunction1_Code : Xbyak::CodeGenerator
            {
                UnknownFormFunction1_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
                {
                    Xbyak::Label retnLabel;

                    // 196070
                    push(rdi);
                    push(r14);
                    push(r15);
                    sub(rsp, 0x20);
                    // 19607A

                    // exit 
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc1HookAddr.GetUIntPtr() + 0xA);
                }
            };

            void *codeBuf = g_localTrampoline.StartAlloc();
            UnknownFormFunction1_Code code(codeBuf);
            g_localTrampoline.EndAlloc(code.getCurr());

            origFunc1 = UnknownFormFunction1_(code.getCode());
            g_branchTrampoline.Write6Branch(origFunc1HookAddr.GetUIntPtr(), GetFnAddr(UnknownFormFunction1));
        }

        {
            struct UnknownFormFunction2_Code : Xbyak::CodeGenerator
            {
                UnknownFormFunction2_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
                {
                    Xbyak::Label retnLabel;

                    // 195DA0
                    mov(ptr[rsp + 0x10], rbx);
                    push(rsi);
                    sub(rsp, 0x20);
                    // 195DAA

                    // exit 
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc2HookAddr.GetUIntPtr() + 0xA);
                }
            };

            void *codeBuf = g_localTrampoline.StartAlloc();
            UnknownFormFunction2_Code code(codeBuf);
            g_localTrampoline.EndAlloc(code.getCurr());

            origFunc2 = UnknownFormFunction2_(code.getCode());
            g_branchTrampoline.Write6Branch(origFunc2HookAddr.GetUIntPtr(), GetFnAddr(UnknownFormFunction2));
        }

        {
            struct UnknownFormFunction3_Code : Xbyak::CodeGenerator
            {
                UnknownFormFunction3_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
                {
                    Xbyak::Label retnLabel;

                    // 196960
                    push(rbp);
                    push(rsi);
                    push(r14);
                    sub(rsp, 0x20);
                    // 196969

                    // exit 
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc3HookAddr.GetUIntPtr() + 0x9);
                }
            };

            void *codeBuf = g_localTrampoline.StartAlloc();
            UnknownFormFunction3_Code code(codeBuf);
            g_localTrampoline.EndAlloc(code.getCurr());

            origFunc3 = UnknownFormFunction3_(code.getCode());
            g_branchTrampoline.Write6Branch(origFunc3HookAddr.GetUIntPtr(), GetFnAddr(UnknownFormFunction3));
        }
        _VMESSAGE("done");

        _VMESSAGE("success");

        return true;
    }
}

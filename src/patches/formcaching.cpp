#include "patches.h"

namespace patches
{
    // treelodreferencecaching.cpp
    void InvalidateCachedForm(std::uint32_t formId);

    tbb::concurrent_hash_map<std::uint32_t, RE::TESForm*> globalFormCacheMap[256];

    REL::Relocation<RE::BSReadWriteLock*> GlobalFormTableLock{ offsets::FormCaching::GlobalFormTableLock };
    REL::Relocation<RE::BSTHashMap<std::uint32_t, RE::TESForm*>**> GlobalFormTable{ offsets::FormCaching::GlobaFormTable };

    typedef void (*UnknownFormFunction0_)(__int64 form, bool a2);
    REL::Relocation<UnknownFormFunction0_> origFunc0HookAddr{ offsets::FormCaching::UnkFormFunc0 };
    UnknownFormFunction0_ origFunc0;

    typedef __int64 (*UnknownFormFunction1_)(__int64 a1, DWORD* formId, __int64* a3);
    REL::Relocation<UnknownFormFunction1_> origFunc1HookAddr{ offsets::FormCaching::UnkFormFunc1 };
    UnknownFormFunction1_ origFunc1;

    typedef __int64 (*UnknownFormFunction2_)(__int64 a1, DWORD* formId, __int64* a3);
    REL::Relocation<UnknownFormFunction2_> origFunc2HookAddr{ offsets::FormCaching::UnkFormFunc2 };
    UnknownFormFunction2_ origFunc2;

    void UpdateFormCache(std::uint32_t FormId, RE::TESForm* Value, bool Invalidate)
    {
        const unsigned char masterId = (FormId & 0xFF000000) >> 24;
        const unsigned int baseId = (FormId & 0x00FFFFFF);

        if (Invalidate)
            globalFormCacheMap[masterId].erase(baseId);
        else
            globalFormCacheMap[masterId].emplace(baseId, Value);

        InvalidateCachedForm(FormId);
    }

    RE::TESForm* hk_GetFormByID(std::uint32_t FormId)
    {
        RE::TESForm* formPointer = nullptr;

        const std::uint8_t masterId = (FormId & 0xFF000000) >> 24;
        const std::uint32_t baseId = (FormId & 0x00FFFFFF);

        {
            std::remove_extent_t<decltype(globalFormCacheMap)>::accessor accessor;

            if (globalFormCacheMap[masterId].find(accessor, baseId))
            {
                formPointer = accessor->second;

                if (formPointer->GetFormID() != FormId) {
                    logger::trace("debug hk_GetFormByID FormId = {:08X}, but formPointer->GetFormID() = {:08X}"sv, FormId, formPointer->GetFormID());
                }
                else
                {
                    return formPointer;
                }
            }
        }

        // Try to use Bethesda's scatter table which is considerably slower
        GlobalFormTableLock->LockForRead();

        if (*GlobalFormTable)
        {
            auto iter = (*GlobalFormTable)->find(FormId);
            formPointer = (iter != (*GlobalFormTable)->end()) ? iter->second : nullptr;
        }

        GlobalFormTableLock->UnlockForRead();

        if (formPointer)
            UpdateFormCache(FormId, formPointer, false);

        return formPointer;
    }

    __int64 UnknownFormFunction2(__int64 a1, DWORD* formId, __int64* a3)
    {
        UpdateFormCache(*formId, nullptr, true);

        return origFunc2(a1, formId, a3);
    }

    __int64 UnknownFormFunction1(__int64 a1, DWORD* formId, __int64* a3)
    {
        UpdateFormCache(*formId, nullptr, true);

        return origFunc1(a1, formId, a3);
    }

    void UnknownFormFunction0(__int64 form, bool a2)
    {
        UpdateFormCache(*(std::uint32_t*)(form + 0x14), nullptr, true);

        origFunc0(form, a2);
    }

    bool PatchFormCaching()
    {
        logger::trace("- form caching -"sv);

        REL::Relocation<std::uint32_t*> LookupFormByID{ offsets::FormCaching::LookupFormByID };
        if (*LookupFormByID != 0x83485740)
        {
            logger::trace("sse fixes is installed and enabled. aborting form cache patch."sv);
            *config::patchFormCaching = false;

            return false;
        }

        logger::trace("detouring GetFormByID"sv);
        SKSE::GetTrampoline().write_branch<6>(LookupFormByID.address(), reinterpret_cast<std::uintptr_t>(hk_GetFormByID));
        logger::trace("done"sv);

        // TODO: write a generic detour instead
        logger::trace("detouring global form table write functions"sv);
        {
            struct UnknownFormFunction0_Code : Xbyak::CodeGenerator
            {
                UnknownFormFunction0_Code()
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
                    dq(origFunc0HookAddr.address() + 0x8);
                }
            };

            UnknownFormFunction0_Code code;
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            origFunc0 = reinterpret_cast<UnknownFormFunction0_>(trampoline.allocate(code));
            trampoline.write_branch<6>(
                origFunc0HookAddr.address(),
                reinterpret_cast<std::uintptr_t>(UnknownFormFunction0));
        }

        {
            struct UnknownFormFunction1_Code : Xbyak::CodeGenerator
            {
                UnknownFormFunction1_Code()
                {
                    Xbyak::Label retnLabel;

                    // 1A0BE0
                    mov(ptr[rsp + 0x10], rbx);
                    mov(ptr[rsp + 0x18], rbp);
                    // 1A0BEA

                    // exit
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc1HookAddr.address() + 0xA);
                }
            };

            UnknownFormFunction1_Code code;
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            origFunc1 = reinterpret_cast<UnknownFormFunction1_>(trampoline.allocate(code));
            trampoline.write_branch<6>(
                origFunc1HookAddr.address(),
                reinterpret_cast<std::uintptr_t>(UnknownFormFunction1));
        }

        {
            struct UnknownFormFunction2_Code : Xbyak::CodeGenerator
            {
                UnknownFormFunction2_Code()
                {
                    Xbyak::Label retnLabel;

                    // 1A17E0
                    mov(ptr[rsp + 0x10], rbx);
                    mov(ptr[rsp + 0x18], rsi);
                    // 1A17EA

                    // exit
                    jmp(ptr[rip + retnLabel]);

                    L(retnLabel);
                    dq(origFunc2HookAddr.address() + 0xA);
                }
            };

            UnknownFormFunction2_Code code;
            code.ready();

            auto& trampoline = SKSE::GetTrampoline();
            origFunc2 = reinterpret_cast<UnknownFormFunction2_>(trampoline.allocate(code));
            trampoline.write_branch<6>(
                origFunc2HookAddr.address(),
                reinterpret_cast<std::uintptr_t>(UnknownFormFunction2));
        }

        logger::trace("done"sv);

        logger::trace("success"sv);

        return true;
    }
}

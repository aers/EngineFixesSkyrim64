#pragma once

#include "tree_lod_reference_caching.h"

// notes on form caching
// 14688 - SetAt - inserts form to map, replaces if it already exists
// 14710 - RemoveAt - removes form from map
// g_FormMap xrefs
// 13689 - TESDataHandler::dtor - clears entire form map, called by TES::dtor, this only happens on game shutdown
// 13754 - TESDataHandler::ClearData - clears entire form map, called on game shutdown but also on Main::PerformGameReset, hook to clear our cache
// 13785 - HotLoadPlugin command handler - no one should be using this, so don't worry about it
// 14593 - TESForm::ctor - calls RemoveAt and SetAt, so handled by those hooks
// 14594 - TESForm::dtor_1 - deallocs the form map if there are zero forms
// 14617 - TESForm::GetFormByNumericId - form lookup, hooked
// 14627 - TESForm::RemoveFromDataStructures - calls RemoveAt, handled by that hook
// 14666 - TESForm::SetFormId - changes formid of form, removes old id from map and adds new one, calls SetAt/RemoveAt, handled by those hooks
// 441564 - TESForm::ReleaseFormDataStructures - deletes form map, inlined in TESForm dtors
// 14669 - TESForm::InitializeFormDataStructures - creates new empty form map, hook to clear our cache
// 14703 - TESForm::dtor_2 - deallocs the form map if there are zero forms
// 22839 - ConsoleFunc::Help - inlined form reader
// 22869 - ConsoleFunc::TestCode - inlined form reader
// 35865 - LoadGameCleanup - inlined form reader

namespace Patches::FormCaching
{
    namespace detail
    {
#ifdef USE_TBB
        using HashMap = tbb::concurrent_hash_map<std::uint32_t, RE::TESForm*>;
#else
        using HashMap = gtl::parallel_flat_hash_map<std::uint32_t, RE::TESForm*>;
#endif

        inline HashMap g_formCache[256];

        inline RE::TESForm* TESForm_GetFormByNumericId(RE::FormID a_formId)
        {
            RE::TESForm* formPointer = nullptr;

            const std::uint8_t masterId = (a_formId & 0xFF000000) >> 24;
            const std::uint32_t baseId = (a_formId & 0x00FFFFFF);

            // lookup form in our cache first
#ifdef USE_TBB
            {
                HashMap::const_accessor a;

                if (g_formCache[masterId].find(a, baseId)) {
                    formPointer = a->second;
                    return formPointer;
                }
            }
#else
            if (g_formCache[masterId].if_contains(baseId, [&formPointer](const HashMap::value_type& v) { formPointer = v.second; }))
            {
                return formPointer;
            }
#endif

            // lookup form in bethesda's map
            formPointer = RE::TESForm::LookupByID(a_formId);

            if (formPointer)
            {
#ifdef USE_TBB
                g_formCache[masterId].emplace(baseId, formPointer);
#else
                g_formCache[masterId].try_emplace_l(baseId, [&formPointer](HashMap::value_type& v) { v.second = formPointer; }, formPointer);
#endif
            }

            return formPointer;
        }

        inline SafetyHookInline g_hk_RemoveAt {};

        inline std::uint64_t FormMap_RemoveAt(RE::BSTHashMap<RE::FormID, RE::TESForm*>* a_self, RE::FormID* a_formIdPtr, void* a_prevValueFunctor)
        {
            const std::uint8_t masterId = (*a_formIdPtr & 0xFF000000) >> 24;
            const std::uint32_t baseId = (*a_formIdPtr & 0x00FFFFFF);

            g_formCache[masterId].erase(baseId);
            TreeLodReferenceCaching::detail::RemoveCachedForm(baseId);

            return g_hk_RemoveAt.call<std::uint64_t>(a_self, a_formIdPtr, a_prevValueFunctor);
        }

        inline SafetyHookInline g_hk_SetAt {};

        // the functor stores the TESForm to set as the first field
        inline std::uint64_t FormScatterTable_SetAt(std::uintptr_t a_self, RE::FormID* a_formIdPtr, RE::TESForm** a_valueFunctor)
        {
            const std::uint8_t masterId = (*a_formIdPtr & 0xFF000000) >> 24;
            const std::uint32_t baseId = (*a_formIdPtr & 0x00FFFFFF);

            RE::TESForm* formPointer = *a_valueFunctor;

#ifdef USE_TBB
            HashMap::accessor a;
            if (!g_formCache[masterId].emplace(a, baseId, formPointer)) {
                logger::trace("replacing an existing form in form cache"sv);
                a->second = formPointer;
            }
#else
            g_formCache[masterId].try_emplace_l(baseId, [&formPointer](HashMap::value_type& v) {
                logger::trace("replacing an existing form in form cache"sv);
                v.second = formPointer;
            }, formPointer);
#endif

            return g_hk_SetAt.call<std::uint64_t>(a_self, a_formIdPtr, a_valueFunctor);
        }

        inline SafetyHookInline g_hk_ClearData;

        // the game does not lock the form table on these clears so we won't either
        // maybe fix later if it causes issues
        inline void TESDataHandler_ClearData(RE::TESDataHandler* a_self)
        {
            for (int i = 0; i < 256; i++)
                g_formCache[i].clear();

            TreeLodReferenceCaching::detail::ClearCache();

            g_hk_ClearData.call(a_self);
        }

        inline SafetyHookInline g_hk_InitializeFormDataStructures;

        inline void TESForm_InitializeFormDataStructures()
        {
            for (int i = 0; i < 256; i++)
                g_formCache[i].clear();

            TreeLodReferenceCaching::detail::ClearCache();

            g_hk_InitializeFormDataStructures.call();
        }

        inline void ReplaceFormMapFunctions()
        {
            REL::Relocation getForm { REL::ID(14617) };
            getForm.replace_func(0x9E, TESForm_GetFormByNumericId);

            const REL::Relocation RemoveAt { REL::ID(14710) };
            g_hk_RemoveAt = safetyhook::create_inline(RemoveAt.address(), FormMap_RemoveAt);

            const REL::Relocation SetAt { REL::ID(14688) };
            g_hk_SetAt = safetyhook::create_inline(SetAt.address(), FormScatterTable_SetAt);

            const REL::Relocation ClearData { REL::ID(13754) };
            g_hk_ClearData = safetyhook::create_inline(ClearData.address(), TESDataHandler_ClearData);

            const REL::Relocation InitializeFormDataStructures { REL::ID(14669) };
            g_hk_InitializeFormDataStructures = safetyhook::create_inline(InitializeFormDataStructures.address(), TESForm_InitializeFormDataStructures);
        }
    }

    inline void Install()
    {
        detail::ReplaceFormMapFunctions();

        logger::info("installed form caching patch"sv);
    }
}
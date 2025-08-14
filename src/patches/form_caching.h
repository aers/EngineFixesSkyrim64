#pragma once

#include <gtl/phmap.hpp>

namespace Patches::FormCaching
{
    namespace detail
    {
        using HashMap = gtl::parallel_flat_hash_map<std::uint32_t, RE::TESForm*, gtl::priv::hash_default_hash<std::uint32_t>, gtl::priv::hash_default_eq<std::uint32_t>, mi_stl_allocator<std::pair<const std::uint32_t, RE::TESForm*>>, 4, std::mutex>;

        inline HashMap g_formCache[256];

        RE::TESForm* TESDataHandler_GetForm(RE::FormID a_formId);

        inline SafetyHookInline g_hk_RemoveFromDataStructures {};

        inline void TESForm_RemoveFromDataStructures(RE::TESForm* a_self, bool a_force)
        {
            if ((a_self->GetFormFlags() & RE::TESForm::RecordFlags::kTemporary) == 0 || a_force)
            {
                const std::uint8_t masterId = (a_self->GetFormID() & 0xFF000000) >> 24;
                const std::uint32_t baseId = (a_self->GetFormID() & 0x00FFFFFF);

                g_formCache[masterId].erase(baseId);
            }

            return g_hk_RemoveFromDataStructures.call(a_self, a_force);
        }

        inline SafetyHookInline g_hk_RemoveAt {};

        inline std::uint64_t FormMap_RemoveAt(RE::BSTHashMap<RE::FormID, RE::TESForm*>* a_self, RE::FormID* a_formIdPtr, void* a_prevValueFunctor)
        {
            const std::uint8_t masterId = (*a_formIdPtr & 0xFF000000) >> 24;
            const std::uint32_t baseId = (*a_formIdPtr & 0x00FFFFFF);

            g_formCache[masterId].erase(baseId);

            return g_hk_RemoveAt.call<std::uint64_t>(a_self, a_formIdPtr, a_prevValueFunctor);
        }

        inline SafetyHookInline g_hk_SetAt {};

        // the functor stores the TESForm to set as the first field
        inline std::uint64_t FormScatterTable_SetAt(std::uintptr_t a_self, RE::FormID* a_formIdPtr, RE::TESForm** a_valueFunctor)
        {
            const std::uint8_t masterId = (*a_formIdPtr & 0xFF000000) >> 24;
            const std::uint32_t baseId = (*a_formIdPtr & 0x00FFFFFF);

            RE::TESForm* formPointer = *a_valueFunctor;

            g_formCache[masterId].try_emplace_l(baseId, [&formPointer](HashMap::value_type& v) { v.second = formPointer; }, formPointer);

            return g_hk_SetAt.call<std::uint64_t>(a_self, a_formIdPtr, a_valueFunctor);
        }

        inline void ReplaceFormMapFunctions()
        {
            REL::Relocation getForm { REL::ID(14617) };
            getForm.replace_func(0x9E, TESDataHandler_GetForm);

            const REL::Relocation RemoveFromDataStructures { REL::ID(14627) };
            g_hk_RemoveFromDataStructures = safetyhook::create_inline(RemoveFromDataStructures.address(), TESForm_RemoveFromDataStructures);

            const REL::Relocation RemoveAt { REL::ID(14710) };
            g_hk_RemoveAt = safetyhook::create_inline(RemoveAt.address(), FormMap_RemoveAt);

            const REL::Relocation SetAt { REL::ID(14688) };
            g_hk_SetAt = safetyhook::create_inline(SetAt.address(), FormScatterTable_SetAt);
        }
    }

    inline void Install()
    {
        detail::ReplaceFormMapFunctions();

        REX::INFO("installed form caching patch");
    }
}
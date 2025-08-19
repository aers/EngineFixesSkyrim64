#pragma once

namespace Patches::TreeLodReferenceCaching
{
    namespace detail
    {
        typedef uint16_t (*Float2Half_)(float f);
        inline REL::Relocation<Float2Half_> Float2Half{ REL::ID(76217) };

#ifdef USE_TBB
        using HashMap = tbb::concurrent_hash_map<std::uint32_t, RE::TESObjectREFR*>;
#else
        using HashMap = gtl::parallel_flat_hash_map<std::uint32_t, RE::TESObjectREFR*>;
#endif

        inline HashMap g_treeReferenceCache;

        void UpdateBlockVisibility(RE::BGSDistantTreeBlock* a_data);

        inline bool HasTreeLod(const RE::TESBoundObject* a_boundObject)
        {
            using STATFlags = RE::TESObjectSTAT::RecordFlags;
            return a_boundObject->GetFormFlags() & STATFlags::kHasTreeLOD || a_boundObject->Is(RE::FormType::Tree);
        }

        inline void RemoveCachedForm(const RE::FormID a_baseId)
        {
            g_treeReferenceCache.erase(a_baseId);
        }

        // when a form gets added to the global cache, check if its a tree and if so erase our cached tree lookup
        // in case this new tree is the correct form
        inline void CheckAndRemoveForm(const std::uint32_t a_baseId, const RE::TESForm* a_form)
        {
            const auto objectReference = a_form->AsReference();
            if (!objectReference)
                return;

            const auto baseObject = objectReference->GetBaseObject();
            if (!baseObject)
                return;

            if (!HasTreeLod(baseObject))
                return;

            // clear cache in case this is our valid tree
            g_treeReferenceCache.erase(a_baseId);
        }

        inline void ClearCache()
        {
            g_treeReferenceCache.clear();
        }

        inline void Install()
        {
            REL::Relocation updateBlockVisibility { REL::ID(31660) };
            updateBlockVisibility.replace_func(0x2C5, UpdateBlockVisibility);
        }
    }

    inline void Install()
    {
        detail::Install();

        logger::info("installed tree lod reference caching patch"sv);
    }
}
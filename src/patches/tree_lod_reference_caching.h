#pragma once

namespace Patches::TreeLodReferenceCaching
{
    namespace detail
    {
        typedef uint16_t (*Float2Half_)(float f);
        inline REL::Relocation<Float2Half_> Float2Half{ RELOCATION_ID(74491, 76217) };

        using HashMap = tbb::concurrent_hash_map<std::uint32_t, RE::TESObjectREFR*>;

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

        inline void ClearCache()
        {
            g_treeReferenceCache.clear();
        }

        inline void Install()
        {
            REL::Relocation updateBlockVisibility{ RELOCATION_ID(30839, 31660) };
            updateBlockVisibility.replace_func(VAR_NUM(0x241, 0x2C5), UpdateBlockVisibility);
        }
    }

    inline void Install()
    {
        detail::Install();

        logger::info("installed tree lod reference caching patch"sv);
    }
}
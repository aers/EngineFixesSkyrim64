#pragma once

#include <oneapi/tbb/concurrent_hash_map.h>

namespace Patches::TreeLodReferenceCaching
{
    namespace detail
    {
        typedef uint16_t (*Float2Half_)(float f);
        inline REL::Relocation<Float2Half_> Float2Half{ REL::ID(76217) };

        //using HashMap = gtl::parallel_flat_hash_map<std::uint32_t, RE::TESObjectREFR*, gtl::priv::hash_default_hash<std::uint32_t>, gtl::priv::hash_default_eq<std::uint32_t>, mi_stl_allocator<std::pair<const std::uint32_t, RE::TESForm*>>, 4, std::mutex>;
        //using HashMap = gtl::parallel_flat_hash_map<std::uint32_t, RE::TESObjectREFR*, gtl::priv::hash_default_hash<std::uint32_t>, gtl::priv::hash_default_eq<std::uint32_t>, tbb::scalable_allocator<std::pair<const std::uint32_t, RE::TESForm*>>, 4, std::mutex>;

        using HashMap = tbb::concurrent_hash_map<std::uint32_t, RE::TESObjectREFR*>;

        inline HashMap g_treeReferenceCache;

        void UpdateBlockVisibility(RE::BGSDistantTreeBlock* a_data);

        inline void RemoveCachedForm(const RE::FormID a_baseId)
        {
            g_treeReferenceCache.erase(a_baseId);
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
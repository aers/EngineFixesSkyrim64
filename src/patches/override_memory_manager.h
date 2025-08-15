#pragma once

namespace Patches::MemoryManager {
    namespace detail {
        inline std::byte* g_Trash { nullptr };

        inline void* Allocate(RE::MemoryManager*, std::size_t a_size, std::uint32_t a_alignment, bool a_alignmentRequired) {
            if (a_size > 0)
                return a_alignmentRequired ? mi_malloc_aligned(a_size, a_alignment) : mi_malloc(a_size);
            return g_Trash;
        }

        inline void* Reallocate(RE::MemoryManager* a_self, void* a_oldMem, std::size_t a_newSize, std::uint32_t a_alignment, bool a_alignmentRequired) {
            if (a_oldMem == g_Trash)
                return Allocate(a_self, a_newSize, a_alignment, a_alignmentRequired);
            else
                return a_alignmentRequired ? mi_realloc_aligned(a_oldMem, a_newSize, a_alignment) : mi_realloc(a_oldMem, a_newSize);
        }

        inline void Deallocate(RE::MemoryManager*, void* a_mem, bool) {
            if (a_mem != g_Trash)
                mi_free(a_mem); // mimalloc has no aligned_free and uses regular free for all malloc calls
        }


        inline void ReplaceAllocRoutines() {
            REL::Relocation allocate{ REL::ID(68115) };
            REL::Relocation reallocate{ REL::ID(68116) };
            REL::Relocation deallocate{ REL::ID(68117) };

            allocate.replace_func(0x248, Allocate);
            reallocate.replace_func(0x1F6, Reallocate);
            deallocate.replace_func(0x114, Deallocate);
        }

        inline void StubInit() {
            REL::Relocation target{ REL::ID(68121) };

            target.write_fill(REL::INT3, 0x1A7);
            target.write(REL::RET);
        }

        inline void Install() {
            StubInit();
            ReplaceAllocRoutines();
            RE::MemoryManager::GetSingleton()->RegisterMemoryManager();
            RE::BSThreadEvent::InitSDM();
        }
    }

    inline void Install() {
        detail::Install();

        REX::INFO("installed global memory manager override patch"sv);
    }
}
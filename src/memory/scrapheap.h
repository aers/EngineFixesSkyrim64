#pragma once

#include "allocator.h"

namespace Memory::ScrapHeap
{
    namespace detail
    {
        // the scrapheap allocator will never allocate less than 0x10 bytes or less than 0x8 alignment
        // this prevents any zero-allocs as in the global memory manager
        inline void* Allocate(RE::ScrapHeap*, std::size_t a_size, std::size_t a_alignment)
        {
            if (a_size < 0x10) {
                a_size = 0x10;
            }
            if (a_alignment < 0x8) {
                a_alignment = 0x8;
            }

            return Allocator::GetAllocator()->AllocateAligned(a_size, a_alignment);
        }

        inline RE::ScrapHeap* Ctor(RE::ScrapHeap* a_self)
        {
            std::memset(a_self, 0, sizeof(RE::ScrapHeap));
            SKSE::stl::emplace_vtable(a_self);
            return a_self;
        }

        inline void Deallocate(RE::ScrapHeap*, void* a_mem)
        {
            Allocator::GetAllocator()->DeallocateAligned(a_mem);
        }

        inline void WriteStubs()
        {
            using tuple_t = std::tuple<REL::ID, std::size_t>;
#ifdef SKYRIM_AE
            constexpr std::array todo{
                tuple_t{ REL::ID(68152), 0xBA },   // Clean
                tuple_t{ REL::ID(68151), 0x8 },    // ClearKeepPages
                tuple_t{ REL::ID(68155), 0xF6 },   // InsertFreeBlock
                tuple_t{ REL::ID(68156), 0x185 },  // RemoveFreeBlock
                tuple_t{ REL::ID(68150), 0x4 },    // SetKeepPages
                tuple_t{ REL::ID(68143), 0x32 },   // dtor
            };
#else
            constexpr std::array todo{
                tuple_t{ REL::ID(66891), 0xC3 },   // Clean
                tuple_t{ REL::ID(66890), 0x8 },    // ClearKeepPages
                tuple_t{ REL::ID(66894), 0xF6 },   // InsertFreeBlock
                tuple_t{ REL::ID(66895), 0x183 },  // RemoveFreeBlock
                tuple_t{ REL::ID(66889), 0x4 },    // SetKeepPages
                tuple_t{ REL::ID(66883), 0x32 },   // dtor
            };
#endif

            for (const auto& [offset, size] : todo) {
                REL::Relocation target{ offset };
                target.write_fill(REL::INT3, size);
                target.write(REL::RET);
            }
        }

        inline void WriteHooks()
        {
            using tuple_t = std::tuple<REL::ID, std::size_t, void*>;
#ifdef SKYRIM_AE
            constexpr std::array todo{
                tuple_t{ REL::ID(68144), 0x5E7, &Allocate },
                tuple_t{ REL::ID(68146), 0x13E, &Deallocate },
                tuple_t{ REL::ID(68142), 0x13A, &Ctor },
            };
#else
            constexpr std::array todo{
                tuple_t{ REL::ID(66884), 0x607, &Allocate },
                tuple_t{ REL::ID(66885), 0x143, &Deallocate },
                tuple_t{ REL::ID(66882), 0x128, &Ctor },
            };
#endif

            for (const auto& [offset, size, func] : todo) {
                REL::Relocation target{ offset };
                target.replace_func(size, func);
            }
        }

        inline void Install()
        {
            WriteStubs();
            WriteHooks();
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed scrapheap patch"sv);
    }
}
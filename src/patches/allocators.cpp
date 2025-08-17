#include "allocators.h"

namespace Patches::Allocators
{
    namespace detail
    {
        std::byte* g_Trash{ nullptr };

        namespace AutoScrapBuffer
        {
            struct Patch final :
                Xbyak::CodeGenerator
            {
                Patch()
                {
                    xor_(rax, rax);
                    cmp(rbx, rax);
                }
            };

            inline void Ctor()
            {
                REL::Relocation target{ REL::ID(68108), 0x1D };
                constexpr std::size_t size = 0x32 - 0x1D;
                target.write_fill(REL::NOP, size);
            }

            inline void Dtor()
            {
                REL::Relocation baseStart{ REL::ID(68109), 0x12 };
                REL::Relocation baseEnd{ REL::ID(68109), 0x2F };

                {
                    constexpr std::size_t size = 0x2F - 0x12;
                    baseStart.write_fill(REL::NOP, size);

                    Patch p;
                    p.ready();
                    assert(p.getSize() <= size);
                    baseStart.write(std::span{ p.getCode<const std::byte*>(), p.getSize() });
                }

                {
                    baseEnd.write(std::uint8_t{ 0x74 });  // jnz -> jz
                }
            }

            inline void Install()
            {
                Ctor();
                Dtor();
            }
        }

        namespace GlobalMemoryManager
        {
            void* Allocate(RE::MemoryManager*, std::size_t a_size, std::uint32_t a_alignment, bool a_alignmentRequired)
            {
                // if (a_size > 0)
                //     return a_alignmentRequired ? _aligned_malloc(a_size, a_alignment) : malloc(a_size);
                // return g_Trash;
                if (a_size > 0)
                    return a_alignmentRequired ? mi_malloc_aligned(a_size, a_alignment) : mi_malloc(a_size);
                return g_Trash;
            }

            void* Reallocate(RE::MemoryManager* a_self, void* a_oldMem, std::size_t a_newSize, std::uint32_t a_alignment, bool a_alignmentRequired)
            {
                // if (a_oldMem == g_Trash)
                //     return Allocate(a_self, a_newSize, a_alignment, a_alignmentRequired);
                // return a_alignmentRequired ? _aligned_realloc(a_oldMem, a_newSize, a_alignment) : realloc(a_oldMem, a_newSize);
                if (a_oldMem == g_Trash)
                    return Allocate(a_self, a_newSize, a_alignment, a_alignmentRequired);
                return a_alignmentRequired ? mi_realloc_aligned(a_oldMem, a_newSize, a_alignment) : mi_realloc(a_oldMem, a_newSize);
            }

            void Deallocate(RE::MemoryManager*, void* a_mem, bool a_alignmentRequired)
            {
                // if (a_mem != g_Trash)
                // {
                //     if (a_alignmentRequired)
                //     {
                //         _aligned_free(a_mem);
                //     }
                //     else
                //     {
                //         free(a_mem);
                //     }
                // }
                if (a_mem != g_Trash)
                {
                    if (a_alignmentRequired)
                    {
                        mi_free(a_mem);
                    }
                    else
                    {
                        mi_free(a_mem);
                    }
                }
            }

            void ReplaceAllocRoutines()
            {
                REL::Relocation allocate{ REL::ID(68115) };
                REL::Relocation reallocate{ REL::ID(68116) };
                REL::Relocation deallocate{ REL::ID(68117) };

                allocate.replace_func(0x248, Allocate);
                reallocate.replace_func(0x1F6, Reallocate);
                deallocate.replace_func(0x114, Deallocate);
            }

            void StubInit()
            {
                REL::Relocation target{ REL::ID(68121) };

                target.write_fill(REL::INT3, 0x1A7);
                target.write(REL::RET);

                REL::Relocation<std::uint32_t*> initFence{ REL::ID(400190) };
                *initFence = 2;
            }

            void Install()
            {
                StubInit();
                ReplaceAllocRoutines();
                RE::MemoryManager::GetSingleton()->RegisterMemoryManager();
                RE::BSThreadEvent::InitSDM();
            }
        }

        namespace ScrapHeap
        {
            void* Allocate(RE::ScrapHeap*, std::size_t a_size, std::size_t a_alignment)
            {
                // return a_size > 0 ? _aligned_malloc(a_size, a_alignment) : g_Trash;
                return a_size > 0 ? mi_malloc_aligned(a_size, a_alignment) : g_Trash;
            }

            RE::ScrapHeap* Ctor(RE::ScrapHeap* a_self)
            {
                std::memset(a_self, 0, sizeof(RE::ScrapHeap));
                REX::EMPLACE_VTABLE(a_self);
                return a_self;
            }

            void Deallocate(RE::ScrapHeap*, void* a_mem)
            {
                // if (a_mem != g_Trash)
                //    _aligned_free(a_mem);
                if (a_mem != g_Trash)
                    mi_free(a_mem);
            }

            void WriteStubs()
            {
                using tuple_t = std::tuple<REL::ID, std::size_t>;
                constexpr std::array todo{
                    tuple_t{ REL::ID(68152), 0xBA },   // Clean
                    tuple_t{ REL::ID(68151), 0x8 },    // ClearKeepPages
                    tuple_t{ REL::ID(68155), 0xF6 },   // InsertFreeBlock
                    tuple_t{ REL::ID(68156), 0x185 },  // RemoveFreeBlock
                    tuple_t{ REL::ID(68150), 0x4 },    // SetKeepPages
                    tuple_t{ REL::ID(68143), 0x32 },   // dtor
                };

                for (const auto& [offset, size] : todo)
                {
                    REL::Relocation target{ offset };
                    target.write_fill(REL::INT3, size);
                    target.write(REL::RET);
                }
            }

            void WriteHooks()
            {
                using tuple_t = std::tuple<REL::ID, std::size_t, void*>;
                constexpr std::array todo{
                    tuple_t{ REL::ID(68144), 0x5E7, &Allocate },
                    tuple_t{ REL::ID(68146), 0x13E, &Deallocate },
                    tuple_t{ REL::ID(68142), 0x13A, &Ctor },
                };

                for (const auto& [offset, size, func] : todo)
                {
                    REL::Relocation target{ offset };
                    target.replace_func(size, func);
                }
            }

            void Install()
            {
                WriteStubs();
                WriteHooks();
            }
        }

        namespace ScaleformAllocator
        {
            class CustomAllocator final :
                public RE::GSysAllocPaged
            {
            public:
                [[nodiscard]] static CustomAllocator* GetSingleton()
                {
                    static CustomAllocator singleton;
                    return std::addressof(singleton);
                }

            protected:
                void GetInfo(Info* a_info) const override
                {
                    assert(a_info != nullptr);

                    a_info->minAlign = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
                    a_info->maxAlign = 0;
                    a_info->granularity = 1u << 16;
                    a_info->sysDirectThreshold = 1;
                    a_info->maxHeapGranularity = 0;
                    a_info->hasRealloc = false;
                }

                void* Alloc(std::size_t a_size, std::size_t a_align) override
                {
                    return _allocator->Allocate(a_size, static_cast<std::uint32_t>(a_align), true);
                }

                bool Free(void* a_ptr, std::size_t, std::size_t) override
                {
                    _allocator->Deallocate(a_ptr, true);
                    return true;
                }

                void* AllocSysDirect(
                    std::size_t a_size,
                    std::size_t a_alignment,
                    std::size_t* a_actualSize,
                    std::size_t* a_actualAlign) override
                {
                    assert(a_actualSize != nullptr);
                    assert(a_actualAlign != nullptr);

                    *a_actualSize = a_size;
                    *a_actualAlign = a_alignment;
                    return _allocator->Allocate(a_size, static_cast<std::uint32_t>(a_alignment), true);
                }

                bool FreeSysDirect(void* a_ptr, std::size_t, std::size_t) override
                {
                    _allocator->Deallocate(a_ptr, true);
                    return true;
                }

            private:
                CustomAllocator() = default;
                CustomAllocator(const CustomAllocator&) = delete;
                CustomAllocator(CustomAllocator&&) = delete;
                ~CustomAllocator() = default;
                CustomAllocator& operator=(const CustomAllocator&) = delete;
                CustomAllocator& operator=(CustomAllocator&&) = delete;

                RE::MemoryManager* _allocator{ RE::MemoryManager::GetSingleton() };
            };

            struct Init
            {
                static void thunk(const RE::GMemoryHeap::HeapDesc& a_rootHeapDesc, RE::GSysAllocBase*)
                {
                    return hook(a_rootHeapDesc, CustomAllocator::GetSingleton());
                }

                static inline REL::Relocation<decltype(thunk)> hook;
            };

            void Install()
            {
                REL::Relocation target{ REL::ID(82323), 0x170 };
                Init::hook = target.write_call<5>(Init::thunk);
            }
        }

        void Install()
        {
            g_Trash = new std::byte[1u << 10]{ static_cast<std::byte>(0) };

            if (Settings::MemoryManager::bOverrideGlobalMemoryManager)
            {
                GlobalMemoryManager::Install();
                REX::INFO("installed global memory manager patch"sv);
            }

            if (Settings::MemoryManager::bOverrideScrapHeap)
            {
                AutoScrapBuffer::Install();
                ScrapHeap::Install();
                REX::INFO("installed scrapheap patch"sv);
            }

            if (Settings::MemoryManager::bOverrideScaleformAllocator)
            {
                ScaleformAllocator::Install();
                REX::INFO("installed scaleform allocator patch"sv);
            }
        }
    }

    void Install()
    {
        detail::Install();
        REX::INFO("installed memory manager patches"sv);
    }
}

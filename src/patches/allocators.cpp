#include "allocators.h"

// #include <tbb/memory_pool.h>

namespace Patches::Allocators
{
    namespace detail
    {
        namespace MemoryManager
        {
            // when the global memory manager has a zero-size allocation it returns scratch memory
            // replicate this behavior
            std::byte* g_ZeroAddress{ nullptr };

            // MemoryManager::AutoScrapBuffer impl
            // ScrapHeap RAII wrapper
            struct AutoScrapBuffer
            {
                static void Ctor(AutoScrapBuffer* a_self, std::size_t a_size, std::size_t a_alignment)
                {
                    if (a_size == 0) {
                        a_self->p_Memory = g_ZeroAddress;
                    }
                    else {
                        a_self->p_Memory = RE::MemoryManager::GetSingleton()->GetThreadScrapHeap()->Allocate(a_size, a_alignment);
                    }
                }

                static void Dtor(AutoScrapBuffer* a_self)
                {
                    if (a_self->p_Memory != g_ZeroAddress) {
                        RE::MemoryManager::GetSingleton()->GetThreadScrapHeap()->Deallocate(a_self->p_Memory);
                    }
                }

                static void Install()
                {
                    REL::Relocation ctor { REL::ID(68108) };
                    REL::Relocation dtor { REL::ID(68109) };

                    ctor.replace_func(0x7B, Ctor);
                    dtor.replace_func(0x54, Dtor);
                }

                void* p_Memory;
            };

            void* Allocate(RE::MemoryManager*, std::size_t a_size, std::uint32_t a_alignment, bool a_alignmentRequired)
            {
                if (a_size == 0) {
                    //logger::info("alloc of size {} detected, caller address {:x}", a_size, reinterpret_cast<std::uintptr_t>(_ReturnAddress()) - REL::Module::get().base());
                    return g_ZeroAddress;
                }
#ifdef USE_TBB
                void* mem = a_alignmentRequired ? scalable_aligned_malloc(a_size, a_alignment) : scalable_malloc(a_size);
#else
                void* mem = a_alignmentRequired ? _aligned_malloc(a_size, a_alignment) : malloc(a_size);
#endif

#ifndef NDEBUG
                if (mem == nullptr) {
                    logger::info("failed to allocate memory, caller address {:x}"sv, reinterpret_cast<std::uintptr_t>(_ReturnAddress()) - REL::Module::get().base() );
                }
#endif
                return mem;
            }

            void* Reallocate(RE::MemoryManager* a_self, void* a_oldMem, std::size_t a_newSize, std::uint32_t a_alignment, bool a_alignmentRequired)
            {
                // if (a_newSize == 0) {
                //     logger::info("alloc of size {} detected, caller address {:x}", a_newSize, reinterpret_cast<std::uintptr_t>(_ReturnAddress()) - REL::Module::get().base());
                // }
                if (a_oldMem == g_ZeroAddress)
                    return Allocate(a_self, a_newSize, a_alignment, a_alignmentRequired);
#ifdef USE_TBB
                void* mem = a_alignmentRequired ? scalable_aligned_realloc(a_oldMem, a_newSize, a_alignment) : scalable_realloc(a_oldMem, a_newSize);
#else
                void* mem = a_alignmentRequired ? _aligned_realloc(a_oldMem, a_newSize, a_alignment) : realloc(a_oldMem, a_newSize);
#endif

#ifndef NDEBUG
                if (mem == nullptr) {
                    logger::info("failed to allocate memory, caller address {:x}"sv, reinterpret_cast<std::uintptr_t>(_ReturnAddress()) - REL::Module::get().base() );
                }
#endif
                return mem;
            }

            void Deallocate(RE::MemoryManager*, void* a_mem, bool a_alignmentRequired)
            {
                if (a_mem != g_ZeroAddress) {
                    if (a_alignmentRequired)
#ifdef USE_TBB
                        scalable_aligned_free(a_mem);
#else
                        _aligned_free(a_mem);
#endif
                    else
#ifdef USE_TBB
                        scalable_free(a_mem);
#else
                        free(a_mem);
#endif
                }
            }

            std::size_t Size(RE::MemoryManager*, void* a_mem)
            {
                if (a_mem == g_ZeroAddress)
                    return 0;

#ifdef USE_TBB
                return scalable_msize(a_mem);
#else
                return _msize(a_mem);
#endif
            }

            void ReplaceAllocRoutines()
            {
                REL::Relocation allocate{ REL::ID(68115) };
                REL::Relocation reallocate{ REL::ID(68116) };
                REL::Relocation deallocate{ REL::ID(68117) };
                REL::Relocation size{ REL::ID(68100) };

                allocate.replace_func(0x248, Allocate);
                reallocate.replace_func(0x1F6, Reallocate);
                deallocate.replace_func(0x114, Deallocate);
                size.replace_func(0x156, Size);
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
                g_ZeroAddress = new std::byte[1u << 10]{ static_cast<std::byte>(0) };

                StubInit();
                ReplaceAllocRoutines();
                RE::MemoryManager::GetSingleton()->RegisterMemoryManager();
                RE::BSThreadEvent::InitSDM();
            }
        }

        namespace ScrapHeap
        {
            // the scrapheap allocator will never allocate less than 0x10 bytes or less than 0x8 alignment
            // this prevents any zero-allocs as in the global memory manager
            void* Allocate(RE::ScrapHeap*, std::size_t a_size, std::size_t a_alignment)
            {
                if (a_size < 0x10) {
                    // logger::info("scrapheap alloc of size {} detected, caller address {:x}", a_size, reinterpret_cast<std::uintptr_t>(_ReturnAddress()) - REL::Module::get().base());
                    a_size = 0x10;
                }
                if (a_alignment < 0x8) {
                    // logger::info("scrapheap alignment of {} detected, caller address {:x}", a_alignment, reinterpret_cast<std::uintptr_t>(_ReturnAddress()) - REL::Module::get().base());
                    a_alignment = 0x8;
                }
#ifdef USE_TBB
                void* mem = scalable_aligned_malloc(a_size, a_alignment);
#else
                void* mem = _aligned_malloc(a_size, a_alignment);
#endif
#ifndef NDEBUG
                if (mem == nullptr) {
                    logger::info("failed to allocate memory, caller address {:x}"sv, reinterpret_cast<std::uintptr_t>(_ReturnAddress()) - REL::Module::get().base() );
                }
#endif
                return mem;
            }

            RE::ScrapHeap* Ctor(RE::ScrapHeap* a_self)
            {
                std::memset(a_self, 0, sizeof(RE::ScrapHeap));
                SKSE::stl::emplace_vtable(a_self);
                return a_self;
            }

            void Deallocate(RE::ScrapHeap*, void* a_mem)
            {
#ifdef USE_TBB
                scalable_aligned_free(a_mem);
#else
                _aligned_free(a_mem);
#endif
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

        namespace CRTAllocator
        {
            void Install() {
#ifdef USE_TBB
                SKSE::PatchIAT(scalable_calloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc");
                SKSE::PatchIAT(scalable_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free");
                SKSE::PatchIAT(scalable_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc");
                SKSE::PatchIAT(scalable_msize, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize");
                SKSE::PatchIAT(scalable_aligned_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free");
                SKSE::PatchIAT(scalable_aligned_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc");
#endif

            }
        }

        void Install()
        {
            if (Settings::MemoryManager::bOverrideMemoryManager.GetValue())
            {
                MemoryManager::Install();
                MemoryManager::AutoScrapBuffer::Install();
                logger::info("installed global memory manager patch"sv);
            }

            if (Settings::MemoryManager::bOverrideScrapHeap.GetValue())
            {
                ScrapHeap::Install();
                logger::info("installed scrapheap patch"sv);
            }

            if (Settings::MemoryManager::bOverrideScaleformAllocator.GetValue())
            {
                ScaleformAllocator::Install();
                logger::info("installed scaleform allocator patch"sv);
            }

            if (Settings::MemoryManager::bOverrideCRTAllocator.GetValue()) {
                CRTAllocator::Install();
                logger::info("installed crt allocator patch"sv);
            }
        }
    }
    void Install()
    {
        detail::Install();
        logger::info("installed memory manager patches"sv);
    }
}

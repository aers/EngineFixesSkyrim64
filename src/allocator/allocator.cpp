#include "allocator.h"

#include <tbb/scalable_allocator.h>

namespace Allocator
{
    class CRTAllocator final : public IAllocator
    {
    public:
        [[nodiscard]] void* Allocate(std::size_t a_size) override
        {
            return malloc(a_size);
        }
        [[nodiscard]] void* AllocateAligned(std::size_t a_size, std::size_t a_alignment) override
        {
            return _aligned_malloc(a_size, a_alignment);
        }
        [[nodiscard]] void* Reallocate(void* a_oldMem, std::size_t a_newSize) override
        {
            return realloc(a_oldMem, a_newSize);
        }
        [[nodiscard]] void* ReallocateAligned(void* a_oldMem, std::size_t a_newSize, std::size_t a_alignment) override
        {
            return _aligned_realloc(a_oldMem, a_newSize, a_alignment);
        }
        size_t Size(void* a_mem) override
        {
            return _msize(a_mem);
        }
        void Deallocate(void* a_mem) override
        {
            free(a_mem);
        }
        void DeallocateAligned(void* a_mem) override
        {
            _aligned_free(a_mem);
        }
    };

    class TBBAllocator final : public IAllocator
    {
        static void log_crt_free(void* a_mem)
        {
            logger::info("tbb_safe_free called original free on memory {:X}"sv, reinterpret_cast<std::uintptr_t>(a_mem));
            free(a_mem);
        }

        static void log_crt_aligned_free(void* a_mem)
        {
            logger::info("tbb_safe_free called original aligned free on memory {:X}"sv, reinterpret_cast<std::uintptr_t>(a_mem));
            _aligned_free(a_mem);
        }
    public:
        [[nodiscard]] void* Allocate(std::size_t a_size) override
        {
            return scalable_malloc(a_size);
        }
        [[nodiscard]] void* AllocateAligned(std::size_t a_size, std::size_t a_alignment) override
        {
            return scalable_aligned_malloc(a_size, a_alignment);
        }
        [[nodiscard]] void* Reallocate(void* a_oldMem, std::size_t a_newSize) override
        {
            return scalable_realloc(a_oldMem, a_newSize);
        }
        [[nodiscard]] void* ReallocateAligned(void* a_oldMem, std::size_t a_newSize, std::size_t a_alignment) override
        {
            return scalable_aligned_realloc(a_oldMem, a_newSize, a_alignment);
        }
        size_t Size(void* a_mem) override
        {
            return scalable_msize(a_mem);
        }
        void Deallocate(void* a_mem) override
        {
            return __TBB_malloc_safer_free(a_mem, log_crt_free);
        }
        void DeallocateAligned(void* a_mem) override
        {
            return __TBB_malloc_safer_free(a_mem, log_crt_aligned_free);
        }
    };

    static IAllocator* _allocator;

    void SetAllocator(const AllocatorKind a_kind)
    {
        switch (a_kind) {
        case CRT:
            _allocator = new CRTAllocator();
            break;
        case TBB:
            _allocator = new TBBAllocator();
            break;
        }
    }

    IAllocator* GetAllocator() { return _allocator; }
}
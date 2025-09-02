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
        void ReplaceImports() override
        {
            logger::info("imports not replaced as they already use the CRT allocator"sv);
        }
    };

    class TBBAllocator final : public IAllocator
    {
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
            return scalable_free(a_mem);
        }
        void DeallocateAligned(void* a_mem) override
        {
            return scalable_aligned_free(a_mem);
        }
        void ReplaceImports() override
        {
            SKSE::PatchIAT(scalable_calloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc");
            SKSE::PatchIAT(scalable_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free");
            SKSE::PatchIAT(scalable_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc");
            SKSE::PatchIAT(scalable_msize, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize");
            SKSE::PatchIAT(scalable_aligned_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free");
            SKSE::PatchIAT(scalable_aligned_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc");
            logger::info("imports replaced with tbb allocator functions");
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
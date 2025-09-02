#pragma once

namespace Allocator
{
    class IAllocator
    {
    public:
        virtual ~IAllocator() = default;
        [[nodiscard]] virtual void* Allocate(std::size_t a_size) = 0;
        [[nodiscard]] virtual void* AllocateAligned(std::size_t a_size, std::size_t a_alignment) = 0;
        [[nodiscard]] virtual void* Reallocate(void* a_oldMem, std::size_t a_newSize) = 0;
        [[nodiscard]] virtual void* ReallocateAligned(void* a_oldMem, std::size_t a_newSize, std::size_t a_alignment) = 0;
        virtual size_t              Size(void* a_mem) = 0;
        virtual void                Deallocate(void* a_mem) = 0;
        virtual void                DeallocateAligned(void* a_mem) = 0;
        virtual void                ReplaceImports() = 0;
    };

    enum AllocatorKind : std::uint8_t
    {
        CRT = 0,
        TBB
    };

    void        SetAllocator(AllocatorKind a_kind);
    IAllocator* GetAllocator();
}
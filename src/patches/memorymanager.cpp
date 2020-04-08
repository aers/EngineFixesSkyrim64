#include "version.h"

#include <cstdlib>

#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/IAT.h"
#include "SKSE/SafeWrite.h"

#include "tbb/scalable_allocator.h"

constexpr REL::ID MemoryManagerAlloc_offset(66859);
constexpr REL::ID MemoryManagerFree_offset(66861);
constexpr REL::ID ScrapHeapInit_offset(66882);
constexpr REL::ID ScrapHeapAlloc_offset(66884);
constexpr REL::ID ScrapHeapFree_offset(66885);
constexpr REL::ID ScrapHeapDeInit_offset(66891);
constexpr REL::ID InitMemoryManager_offset(35202);
constexpr REL::ID InitBSSmallBlockAllocator_offset(35201);

class MemoryManager
{
public:
    void* Alloc(std::size_t a_size, std::uint32_t a_alignment, bool a_aligned);
    void Free(void* a_memory, bool a_aligned);

private:
    MemoryManager() = delete;
    ~MemoryManager() = delete;
};

class ScrapHeap
{
public:
    void* Alloc(std::size_t a_size, std::uint32_t a_alignment);
    void Free(void* a_memory);

private:
    ScrapHeap() = delete;
    ~ScrapHeap() = delete;

    static constexpr std::uint32_t MAX_ALLOC_SIZE = 0x4000000;
};

void* proxy_tbbmalloc(std::size_t a_size, std::size_t a_alignment = 0, bool a_aligned = false, bool a_zeroed = false)
{
    if (!a_aligned)
    {
        a_alignment = 4;
    }

    if (a_size <= 0)
    {
        a_size = 1;
        a_alignment = 2;
    }

    // Must be a power of 2, round it up if needed
    if ((a_alignment & (a_alignment - 1)) != 0)
    {
        --a_alignment;
        a_alignment |= a_alignment >> 1;
        a_alignment |= a_alignment >> 2;
        a_alignment |= a_alignment >> 4;
        a_alignment |= a_alignment >> 8;
        a_alignment |= a_alignment >> 16;
        ++a_alignment;
    }

    // Size must be a multiple of alignment, round up to nearest
    if ((a_size % a_alignment) != 0)
    {
        a_size = ((a_size + a_alignment - 1) / a_alignment) * a_alignment;
    }

    void* ptr = scalable_aligned_malloc(a_size, a_alignment);

    if (ptr && a_zeroed)
    {
        return std::memset(ptr, 0, a_size);
    }

    return ptr;
}

void proxy_tbbfree(void* a_memory)
{
    if (a_memory)
    {
        scalable_aligned_free(a_memory);
    }
}

//
// VS2015 CRT hijacked functions
//
void* __fastcall hk_calloc(size_t a_count, size_t a_size)
{
    // The allocated memory is always zeroed
    return proxy_tbbmalloc(a_count * a_size, 0, false, true);
}

void* __fastcall hk_malloc(size_t a_size)
{
    return proxy_tbbmalloc(a_size);
}

void* __fastcall hk_aligned_malloc(size_t a_size, size_t a_alignment)
{
    return proxy_tbbmalloc(a_size, a_alignment, true);
}

void __fastcall hk_free(void* a_block)
{
    proxy_tbbfree(a_block);
}

void __fastcall hk_aligned_free(void* a_block)
{
    proxy_tbbfree(a_block);
}

size_t __fastcall hk_msize(void* a_block)
{
    return scalable_msize(a_block);
}

//
// Internal engine heap allocators backed by VirtualAlloc()
//
void* MemoryManager::Alloc(std::size_t a_size, std::uint32_t a_alignment, bool a_aligned)
{
    return proxy_tbbmalloc(a_size, a_alignment, a_aligned, true);
}

void MemoryManager::Free(void* a_memory, [[maybe_unused]] bool a_aligned)
{
    proxy_tbbfree(a_memory);
}

void* ScrapHeap::Alloc(std::size_t a_size, std::uint32_t a_alignment)
{
    if (a_size > MAX_ALLOC_SIZE || a_size < 0)
    {
        _MESSAGE("warning: scrapheap alloc size %d out of default bounds detected", a_size);
    }

    return proxy_tbbmalloc(a_size, a_alignment, a_alignment != 0);
}

void ScrapHeap::Free(void* Memory)
{
    proxy_tbbfree(Memory);
}

namespace patches
{
    class SafeExit
    {
    public:
        static void Install()
        {
            _VMESSAGE("using safe exit");

            REL::Offset<std::uintptr_t> target = REL::ID(35545);  // Main::Shutdown
            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write5Call(target.GetAddress() + 0x35, Shutdown);
        }

    private:
        static void Shutdown()
        {
            _VMESSAGE("executing safe exit");

            std::exit(EXIT_SUCCESS);
        }
    };

    bool PatchMemoryManager()
    {
        _VMESSAGE("-- memory manager --");

        SafeExit::Install();

        _VMESSAGE("enabling OS allocator use");

        REL::Offset<std::uintptr_t> InitMemoryManager(InitMemoryManager_offset);
        REL::Offset<std::uintptr_t> InitBSSmallBlockAllocator(InitBSSmallBlockAllocator_offset);

        SKSE::SafeWrite8(InitMemoryManager.GetAddress(), 0xC3);          // [3GB] MemoryManager - Default/Static/File heaps
        SKSE::SafeWrite8(InitBSSmallBlockAllocator.GetAddress(), 0xC3);  // [1GB] BSSmallBlockAllocator

        _VMESSAGE("success");

        return true;
    }

    bool PatchTBBMalloc()
    {
        _VMESSAGE("using tbbmalloc");

        REL::Offset<std::uintptr_t> MemoryManagerAlloc(MemoryManagerAlloc_offset);
        REL::Offset<std::uintptr_t> MemoryManagerFree(MemoryManagerFree_offset);
        REL::Offset<std::uintptr_t> ScrapHeapInit(ScrapHeapInit_offset);
        REL::Offset<std::uintptr_t> ScrapHeapAlloc(ScrapHeapAlloc_offset);
        REL::Offset<std::uintptr_t> ScrapHeapFree(ScrapHeapFree_offset);
        REL::Offset<std::uintptr_t> ScrapHeapDeInit(ScrapHeapDeInit_offset);

        SKSE::SafeWrite8(ScrapHeapInit.GetAddress(), 0xC3);    // [64MB ] ScrapHeap init
        SKSE::SafeWrite8(ScrapHeapDeInit.GetAddress(), 0xC3);  // [64MB ] ScrapHeap deinit

        _VMESSAGE("patching CRT IAT memory functions");
        SKSE::PatchIAT(hk_calloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc");
        SKSE::PatchIAT(hk_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc");
        SKSE::PatchIAT(hk_aligned_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc");
        SKSE::PatchIAT(hk_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free");
        SKSE::PatchIAT(hk_aligned_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free");
        SKSE::PatchIAT(hk_msize, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize");

        _VMESSAGE("redirecting memory manager alloc and free");
        auto trampoline = SKSE::GetTrampoline();
        trampoline->Write6Branch(MemoryManagerAlloc.GetAddress(), &MemoryManager::Alloc);
        trampoline->Write6Branch(MemoryManagerFree.GetAddress(), &MemoryManager::Free);
        trampoline->Write6Branch(ScrapHeapAlloc.GetAddress(), &ScrapHeap::Alloc);
        trampoline->Write6Branch(ScrapHeapFree.GetAddress(), &ScrapHeap::Free);

        _VMESSAGE("success");

        return true;
    }
}

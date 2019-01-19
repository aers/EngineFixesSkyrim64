#include "tbb/scalable_allocator.h"

#include "skse64_common/Utilities.h"

#include "patches.h"
#include "utils.h"

namespace patches
{
    class MemoryManager
    {
    private:
        MemoryManager() {}
        ~MemoryManager() {}

    public:
        void *Alloc(size_t Size, uint32_t Alignment, bool Aligned);
        void Free(void *Memory, bool Aligned);
    };

    class ScrapHeap
    {
    private:
        ScrapHeap() {}
        ~ScrapHeap() {}

    public:
        const static uint32_t MAX_ALLOC_SIZE = 0x4000000;

        void *Alloc(size_t Size, uint32_t Alignment);
        void Free(void *Memory);
    };


    RelocAddr <uintptr_t> MemoryManagerAlloc(MemoryManagerAlloc_offset);
    RelocAddr <uintptr_t> MemoryManagerFree(MemoryManagerFree_offset);
    RelocAddr <uintptr_t> ScrapHeapInit(ScrapHeapInit_offset);
    RelocAddr <uintptr_t> ScrapHeapAlloc(ScrapHeapAlloc_offset);
    RelocAddr <uintptr_t> ScrapHeapFree(ScrapHeapFree_offset);
    RelocAddr <uintptr_t> ScrapHeapDeInit(ScrapHeapDeInit_offset);
    RelocAddr <uintptr_t> InitMemoryManager(InitMemoryManager_offset);
    RelocAddr <uintptr_t> InitBSSmallBlockAllocator(InitBSSmallBlockAllocator_offset);


    void *proxy_tbbmalloc(size_t Size, size_t Alignment = 0, bool Aligned = false, bool Zeroed = false)
    {
        if (!Aligned)
            Alignment = 4;

        if (Size <= 0)
        {
            Size = 1;
            Alignment = 2;
        }


        // Must be a power of 2, round it up if needed
        if ((Alignment & (Alignment - 1)) != 0)
        {
            Alignment--;
            Alignment |= Alignment >> 1;
            Alignment |= Alignment >> 2;
            Alignment |= Alignment >> 4;
            Alignment |= Alignment >> 8;
            Alignment |= Alignment >> 16;
            Alignment++;
        }

        // Size must be a multiple of alignment, round up to nearest
        if ((Size % Alignment) != 0)
            Size = ((Size + Alignment - 1) / Alignment) * Alignment;

        void *ptr = scalable_aligned_malloc(Size, Alignment);

        if (ptr && Zeroed)
            return memset(ptr, 0, Size);

        return ptr;
    }

    void proxy_tbbfree(void *Memory)
    {
        if (!Memory)
            return;

        scalable_aligned_free(Memory);
    }

    //
    // VS2015 CRT hijacked functions
    //
    void *__fastcall hk_calloc(size_t Count, size_t Size)
    {
        // The allocated memory is always zeroed
        return proxy_tbbmalloc(Count * Size, 0, false, true);
    }

    void *__fastcall hk_malloc(size_t Size)
    {
        return proxy_tbbmalloc(Size);
    }

    void *__fastcall hk_aligned_malloc(size_t Size, size_t Alignment)
    {
        return proxy_tbbmalloc(Size, Alignment, true);
    }

    void __fastcall hk_free(void *Block)
    {
        proxy_tbbfree(Block);
    }

    void __fastcall hk_aligned_free(void *Block)
    {
        proxy_tbbfree(Block);
    }

    size_t __fastcall hk_msize(void *Block)
    {
        return scalable_msize(Block);
    }

    char *__fastcall hk_strdup(const char *str1)
    {
        size_t len = (strlen(str1) + 1) * sizeof(char);
        return (char *)memcpy(hk_malloc(len), str1, len);
    }

    //
    // Internal engine heap allocators backed by VirtualAlloc()
    //
    void *MemoryManager::Alloc(size_t Size, uint32_t Alignment, bool Aligned)
    {
        return proxy_tbbmalloc(Size, Alignment, Aligned, true);
    }

    void MemoryManager::Free(void *Memory, bool Aligned)
    {
        proxy_tbbfree(Memory);
    }

    void *ScrapHeap::Alloc(size_t Size, uint32_t Alignment)
    {
        if (Size > MAX_ALLOC_SIZE)
            return nullptr;

        return proxy_tbbmalloc(Size, Alignment, Alignment != 0);
    }

    void ScrapHeap::Free(void *Memory)
    {
        proxy_tbbfree(Memory);
    }

    bool PatchMemoryManager()
    {
        _VMESSAGE("-- memory manager --");

        _VMESSAGE("enabling OS allocator use");

        SafeWrite8(InitMemoryManager.GetUIntPtr(), 0xC3);			// [3GB] MemoryManager - Default/Static/File heaps
        SafeWrite8(InitBSSmallBlockAllocator.GetUIntPtr(), 0xC3);	// [1GB] BSSmallBlockAllocator

        if (config::experimentalUseTBBMalloc)
        {
            _VMESSAGE("using tbbmalloc");

            SafeWrite8(ScrapHeapInit.GetUIntPtr(), 0xC3);				// [64MB ] ScrapHeap init
            SafeWrite8(ScrapHeapDeInit.GetUIntPtr(), 0xC3);				// [64MB ] ScrapHeap deinit

            _VMESSAGE("patching CRT IAT memory functions");
            PatchIAT(GetFnAddr(hk_calloc), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc");
            PatchIAT(GetFnAddr(hk_malloc), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc");
            PatchIAT(GetFnAddr(hk_aligned_malloc), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc");
            PatchIAT(GetFnAddr(hk_free), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free");
            PatchIAT(GetFnAddr(hk_aligned_free), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free");
            PatchIAT(GetFnAddr(hk_msize), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize");
            PatchIAT(GetFnAddr(hk_strdup), "API-MS-WIN-CRT-STRING-L1-1-0.DLL", "_strdup");

            _VMESSAGE("redirecting memory manager alloc and free");
            g_branchTrampoline.Write6Branch(MemoryManagerAlloc.GetUIntPtr(), GetFnAddr(&MemoryManager::Alloc));
            g_branchTrampoline.Write6Branch(MemoryManagerFree.GetUIntPtr(), GetFnAddr(&MemoryManager::Free));
            g_branchTrampoline.Write6Branch(ScrapHeapAlloc.GetUIntPtr(), GetFnAddr(&ScrapHeap::Alloc));
            g_branchTrampoline.Write6Branch(ScrapHeapFree.GetUIntPtr(), GetFnAddr(&ScrapHeap::Free));
        }
        _VMESSAGE("success");

        return true;
    }
}
#include <jemalloc/jemalloc.h>
#include "MemoryManager.h"
#include "../util.h"
#include <skse64_common/BranchTrampoline.h>


namespace MemoryManager
{
	// E8 ? ? ? ? 89 38  ->
	RelocAddr <uintptr_t> HeapAlloc(0x00C02770);
	// E8 ? ? ? ? 89 77 0C ->
	RelocAddr <uintptr_t> HeapFree(0x00C02A70);
	// E8 ? ? ? ? 90 0F AE F0 +0x102
	RelocAddr <uintptr_t> InitMemoryManager(0x0059BAA0);
	RelocAddr <uintptr_t> InitBSSmallBlockAllocator(0x0059B6D0);
	//
	// VS2015 CRT hijacked functions
	//
	MemoryManager fakeManager;

	void *__fastcall hk_calloc(size_t Count, size_t Size)
	{
		// The allocated memory is always zeroed
		return fakeManager.Alloc(Count * Size, 0, false);
	}

	void *__fastcall hk_malloc(size_t Size)
	{
		return fakeManager.Alloc(Size, 0, false);
	}

	void *__fastcall hk_aligned_malloc(size_t Size, size_t Alignment)
	{
		return fakeManager.Alloc(Size, (int)Alignment, true);
	}

	void __fastcall hk_free(void *Block)
	{
		return fakeManager.Free(Block, false);
	}

	void __fastcall hk_aligned_free(void *Block)
	{
		return fakeManager.Free(Block, true);
	}

	size_t __fastcall hk_msize(void *Block)
	{
		return je_malloc_usable_size(Block);
	}

	//
	// Internal engine heap allocator backed by VirtualAlloc()
	//
	void *MemoryManager::Alloc(size_t Size, uint32_t Alignment, bool Aligned)
	{

		void *ptr = nullptr;

		// Does this need to be on a certain boundary?
		if (Aligned)
		{

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

			ptr = je_aligned_alloc(Alignment, Size);
		}
		else
		{
			// Normal allocation
			ptr = je_malloc(Size);
		}

		if (!ptr)
			return nullptr;

		return memset(ptr, 0, Size);
	}

	void MemoryManager::Free(void *Memory, bool Aligned)
	{
		if (!Memory)
			return;

		je_free(Memory);
	}

	bool Patch()
	{
		_MESSAGE("-- memory manager --");
		bool option;
		option = true;  je_mallctl("background_thread", nullptr, nullptr, &option, sizeof(bool));
		option = false; je_mallctl("prof.active", nullptr, nullptr, &option, sizeof(bool));

		_MESSAGE("patching CRT IAT memory functions");
		PatchIAT(GetFnAddr(hk_calloc), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc");
		PatchIAT(GetFnAddr(hk_malloc), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc");
		PatchIAT(GetFnAddr(hk_aligned_malloc), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc");
		PatchIAT(GetFnAddr(hk_free), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free");
		PatchIAT(GetFnAddr(hk_aligned_free), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free");
		PatchIAT(GetFnAddr(hk_msize), "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize");

		_MESSAGE("enabling OS allocator use");

		SafeWrite8(InitMemoryManager.GetUIntPtr(), 0xC3);			// [3GB] MemoryManager - Default/Static/File heaps
		SafeWrite8(InitBSSmallBlockAllocator.GetUIntPtr(), 0xC3);	// [1GB] BSSmallBlockAllocator
																	// [128MB] BSScaleformSysMemMapper is untouched due to complexity
																	// [512MB] hkMemoryAllocator is untouched due to complexity

		_MESSAGE("redirecting memory manager alloc and free");
		g_branchTrampoline.Write6Branch(HeapAlloc.GetUIntPtr(), GetFnAddr(&MemoryManager::Alloc));
		g_branchTrampoline.Write6Branch(HeapFree.GetUIntPtr(), GetFnAddr(&MemoryManager::Free));

		_MESSAGE("success");

		return true;
	}
}

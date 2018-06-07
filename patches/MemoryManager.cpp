#include <jemalloc/jemalloc.h>
#include "MemoryManager.h"
#include "../util.h"
#include <skse64_common/BranchTrampoline.h>
#include "../../skse64/GameForms.h"
#include "../../skse64/GameData.h"
#include "../TES/BGSShaderParticleGeometryData.h"


namespace MemoryManager
{
	// E8 ? ? ? ? 89 38  ->
	RelocAddr <uintptr_t> MemoryManagerAlloc(0x00C02770);
	// E8 ? ? ? ? 89 77 0C ->
	RelocAddr <uintptr_t> MemoryManagerFree(0x00C02A70);
	// relative offsets to these are unlikely to change
	RelocAddr <uintptr_t> ScrapHeapInit(0x00C03830);
	RelocAddr <uintptr_t> ScrapHeapAlloc(0x00C039B0);
	RelocAddr <uintptr_t> ScrapHeapFree(0x00C03FD0);
	RelocAddr <uintptr_t> ScrapHeapDeInit(0x00C04190);
	// E8 ? ? ? ? 90 0F AE F0 +0x102
	RelocAddr <uintptr_t> InitMemoryManager(0x0059BAA0);
	RelocAddr <uintptr_t> InitBSSmallBlockAllocator(0x0059B6D0);

	// temporary particle fix
	typedef bool(*BGSShaderParticleGeometryData_LoadForm_)(TES::BGSShaderParticleGeometryData * thisPtr, ModInfo * modInfo);
	BGSShaderParticleGeometryData_LoadForm_ orig_BGSShaderParticleGeometryData_LoadForm;
	RelocPtr<BGSShaderParticleGeometryData_LoadForm_> vtbl_BGSShaderParticleGeometryData_LoadForm(0x01579FF0); // vtbl[6]

	bool hk_BGSShaderParticleGeometryData_LoadForm(TES::BGSShaderParticleGeometryData * thisPtr, ModInfo * modInfo)
	{
		const bool retVal = orig_BGSShaderParticleGeometryData_LoadForm(thisPtr, modInfo);

		if (thisPtr->data.count >= 12)
		{
			const auto particleDensity = thisPtr->data[11];
			if (particleDensity > 10.0)
				thisPtr->data[11] = 10.0f;
		}

		return retVal;
	}

	void *Jemalloc(size_t Size, size_t Alignment = 0, bool Aligned = false, bool Zeroed = false)
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

		if (ptr && Zeroed)
			return memset(ptr, 0, Size);

		return ptr;
	}

	void Jefree(void *Memory)
	{
		if (!Memory)
			return;

		je_free(Memory);
	}

	//
	// VS2015 CRT hijacked functions
	//
	void *__fastcall hk_calloc(size_t Count, size_t Size)
	{
		// The allocated memory is always zeroed
		return Jemalloc(Count * Size, 0, false, true);
	}

	void *__fastcall hk_malloc(size_t Size)
	{
		return Jemalloc(Size);
	}

	void *__fastcall hk_aligned_malloc(size_t Size, size_t Alignment)
	{
		return Jemalloc(Size, Alignment, true);
	}

	void __fastcall hk_free(void *Block)
	{
		Jefree(Block);
	}

	void __fastcall hk_aligned_free(void *Block)
	{
		Jefree(Block);
	}

	size_t __fastcall hk_msize(void *Block)
	{
		return je_malloc_usable_size(Block);
	}

	//
	// Internal engine heap allocators backed by VirtualAlloc()
	//
	void *MemoryManager::Alloc(size_t Size, uint32_t Alignment, bool Aligned)
	{
		return Jemalloc(Size, Alignment, Aligned, true);
	}

	void MemoryManager::Free(void *Memory, bool Aligned)
	{
		Jefree(Memory);
	}

	void *ScrapHeap::Alloc(size_t Size, uint32_t Alignment)
	{
		if (Size > MAX_ALLOC_SIZE)
			return nullptr;

		return Jemalloc(Size, Alignment, Alignment != 0);
	}

	void ScrapHeap::Free(void *Memory)
	{
		Jefree(Memory);
	}

	bool Patch()
	{
		_MESSAGE("-- memory manager --");

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
		SafeWrite8(ScrapHeapInit.GetUIntPtr(), 0xC3);				// [64MB ] ScrapHeap init
		SafeWrite8(ScrapHeapDeInit.GetUIntPtr(), 0xC3);				// [64MB ] ScrapHeap deinit

		_MESSAGE("redirecting memory manager alloc and free");
		g_branchTrampoline.Write6Branch(MemoryManagerAlloc.GetUIntPtr(), GetFnAddr(&MemoryManager::Alloc));
		g_branchTrampoline.Write6Branch(MemoryManagerFree.GetUIntPtr(), GetFnAddr(&MemoryManager::Free));
		g_branchTrampoline.Write6Branch(ScrapHeapAlloc.GetUIntPtr(), GetFnAddr(&ScrapHeap::Alloc));
		g_branchTrampoline.Write6Branch(ScrapHeapFree.GetUIntPtr(), GetFnAddr(&ScrapHeap::Free));

		// temp particle fix
		orig_BGSShaderParticleGeometryData_LoadForm = *vtbl_BGSShaderParticleGeometryData_LoadForm;
		SafeWrite64(vtbl_BGSShaderParticleGeometryData_LoadForm.GetUIntPtr(), GetFnAddr(hk_BGSShaderParticleGeometryData_LoadForm));
		_MESSAGE("success");

		return true;
	}
}

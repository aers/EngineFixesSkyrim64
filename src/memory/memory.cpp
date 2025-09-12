#include "memory.h"
#include "allocator.h"
#include "havok_memory_system.h"
#include "memory_manager.h"
#include "renderpass_cache.h"
#include "scaleform_allocator.h"
#include "scrapheap.h"

namespace Memory
{
    void Install()
    {
        if (Settings::Debug::bDisableTBB.GetValue()) {
            Allocator::SetAllocator(Allocator::CRT);
            logger::info("set allocator to CRT"sv);
        } else {
            Allocator::SetAllocator(Allocator::TBB);
            logger::info("set allocator to TBB"sv);
        }

        if (Settings::MemoryManager::bOverrideMemoryManager.GetValue())
            MemoryManager::Install();

        if (Settings::MemoryManager::bOverrideScrapHeap.GetValue())
            ScrapHeap::Install();

        if (Settings::MemoryManager::bOverrideScaleformAllocator.GetValue())
            ScaleformAllocator::Install();

        if (Settings::MemoryManager::bOverrideRenderPassCache.GetValue())
            RenderPassCache::Install();

        if (Settings::MemoryManager::bOverrideHavokMemorySystem.GetValue())
            HavokMemorySystem::Install();

        if (Settings::MemoryManager::bReplaceImports.GetValue())
            Allocator::GetAllocator()->ReplaceImports();
    }
}
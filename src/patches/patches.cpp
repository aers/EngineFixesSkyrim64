#include "patches.h"

#include "memory_manager.h"
#include "override_crt_allocator.h"

namespace Patches {
    void PreLoad() {
        if (Settings::MemoryManager::bOverrideCRTAllocator)
            OverrideCRTAllocator::Install();

        if (Settings::MemoryManager::bOverrideGlobalMemoryManager)
            MemoryManager::Install();
    }
}
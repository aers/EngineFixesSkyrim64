#include "patches.h"

#include "form_caching.h"
#include "override_crt_allocator.h"
#include "override_memory_manager.h"

namespace Patches {
    void PreLoad() {
        if (Settings::MemoryManager::bOverrideCRTAllocator)
            OverrideCRTAllocator::Install();

        if (Settings::MemoryManager::bOverrideGlobalMemoryManager)
            MemoryManager::Install();
    }

    void Load()
    {
        if (Settings::Patches::bFormCaching)
            FormCaching::Install();
    }
}
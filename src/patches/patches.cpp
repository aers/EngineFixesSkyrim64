#include "patches.h"

#include "disable_chargen_precache.h"
#include "enable_achievements.h"
#include "form_caching.h"
#include "max_stdio.h"
#include "override_crt_allocator.h"
#include "override_memory_manager.h"
#include "tree_lod_reference_caching.h"

namespace Patches {
    void PreLoad() {
        if (Settings::MemoryManager::bOverrideCRTAllocator)
            OverrideCRTAllocator::Install();

        if (Settings::MemoryManager::bOverrideGlobalMemoryManager)
            MemoryManager::Install();
    }

    void Load()
    {
        if (Settings::Patches::bDisableChargenPrecache)
            DisableChargenPrecache::Install();

        if (Settings::Patches::bEnableAchievementsWithMods)
            EnableAchievementsWithMods::Install();

        if (Settings::Patches::bFormCaching)
            FormCaching::Install();

        if (Settings::Patches::bMaxStdIO > 512)
            MaxStdIO::Install();

        if (Settings::Patches::bFormCaching && Settings::Patches::bTreeLodReferenceCaching)
            TreeLodReferenceCaching::Install();
    }
}
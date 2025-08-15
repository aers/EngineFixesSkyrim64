#include "patches.h"

#include "disable_chargen_precache.h"
#include "enable_achievements.h"
#include "form_caching.h"
#include "max_stdio.h"
#include "override_crt_allocator.h"
#include "override_memory_manager.h"
#include "override_scaleform_allocator.h"
#include "override_scrapheap.h"
#include "regular_quicksaves.h"
#include "safe_exit.h"
#include "save_added_sound_categories.h"
#include "scrolling_doesnt_switch_pov.h"
#include "sleep_wait_time.h"
#include "tree_lod_reference_caching.h"
#include "waterflow_animation.h"

namespace Patches {
    void PreLoad() {
        if (Settings::Patches::bMemoryManager)
        {
            if (Settings::MemoryManager::bOverrideCRTAllocator)
                OverrideCRTAllocator::Install();

            if (Settings::MemoryManager::bOverrideGlobalMemoryManager)
                OverrideMemoryManager::Install();

            if (Settings::MemoryManager::bOverrideScrapHeap)
                OverrideScrapHeap::Install();

            if (Settings::MemoryManager::bOverrideScaleformAllocator)
                OverrideScaleformAllocator::Install();
        }

        if (Settings::Patches::bSafeExit)
            SafeExit::Install();
    }

    void Load()
    {
        if (Settings::Patches::bDisableChargenPrecache)
            DisableChargenPrecache::Install();

        if (Settings::Patches::bEnableAchievementsWithMods)
            EnableAchievementsWithMods::Install();

        if (Settings::Patches::bFormCaching)
            FormCaching::Install();

        if (Settings::Patches::iMaxStdIO > 512)
            MaxStdIO::Install();

        if (Settings::Patches::bRegularQuicksaves)
            RegularQuicksaves::Install();

        if (Settings::Patches::bSaveAddedSoundCategories)
            SaveAddedSoundCategories::Install();

        if (Settings::Patches::bScrollingDoesntSwitchPOV)
            ScrollingDoesntSwitchPOV::Install();

        if (Settings::Patches::bSleepWaitTime)
            SleepWaitTime::Install();

        if (Settings::Patches::bFormCaching && Settings::Patches::bTreeLodReferenceCaching)
            TreeLodReferenceCaching::Install();

        if (Settings::Patches::bWaterflowAnimation)
            WaterflowAnimation::Install();
    }
}
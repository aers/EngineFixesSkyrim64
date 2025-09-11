#include "patches.h"

#include "allocators.h"
#include "disable_chargen_precache.h"
#include "disable_snow_flag.h"
#include "enable_achievements.h"
#include "form_caching.h"
#include "ini_setting_collection.h"
#include "max_stdio.h"
#include "regular_quicksaves.h"
#include "renderpass_cache.h"
#include "safe_exit.h"
#include "save_added_sound_categories.h"
#include "save_game_max_size.h"
#include "scaleform_allocator.h"
#include "scrolling_doesnt_switch_pov.h"
#include "sleep_wait_time.h"
#include "tree_lod_reference_caching.h"
#include "waterflow_animation.h"

namespace Patches
{
    void Load()
    {
        Allocators::Install();

        if (Settings::MemoryManager::bOverrideScaleformAllocator.GetValue())
            ScaleformAllocator::Install();

        if (Settings::MemoryManager::bOverrideRenderPassCache.GetValue())
            RenderPassCache::Install();

        if (Settings::MemoryManager::bReplaceImports.GetValue())
            Allocator::GetAllocator()->ReplaceImports();

        if (Settings::Patches::bDisableChargenPrecache.GetValue())
            DisableChargenPrecache::Install();

        if (Settings::Patches::bDisableSnowFlag.GetValue())
            DisableSnowFlag::Install();

        if (Settings::Patches::bEnableAchievementsWithMods.GetValue())
            EnableAchievementsWithMods::Install();

        if (Settings::Patches::bFormCaching.GetValue())
            FormCaching::Install();

        if (Settings::Patches::bINISettingCollection.GetValue())
            INISettingCollection::Install();

        if (Settings::Patches::bMaxStdIO.GetValue())
            MaxStdIO::Install();

        if (Settings::Patches::bRegularQuicksaves.GetValue())
            RegularQuicksaves::Install();

        if (Settings::Patches::bSafeExit.GetValue())
            SafeExit::Install();

        if (Settings::Patches::bSaveAddedSoundCategories.GetValue())
            SaveAddedSoundCategories::Install();

        if (Settings::Patches::iSaveGameMaxSize.GetValue() != 64)
            SaveGameMaxSize::Install();

        if (Settings::Patches::bScrollingDoesntSwitchPOV.GetValue())
            ScrollingDoesntSwitchPOV::Install();

        if (Settings::Patches::bSleepWaitTime.GetValue())
            SleepWaitTime::Install();

        if (Settings::Patches::bFormCaching.GetValue() && Settings::Patches::bTreeLodReferenceCaching.GetValue())
            TreeLodReferenceCaching::Install();

        if (Settings::Patches::bWaterflowAnimation.GetValue())
            WaterflowAnimation::Install();
    }
}
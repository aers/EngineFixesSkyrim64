#include "patches.h"

namespace patches
{
    bool PatchAll()
    {
        if (config::patchEnableAchievementsWithMods)
            PatchDisableChargenPrecache();

        if (config::patchEnableAchievementsWithMods)
            PatchEnableAchievementsWithMods();

        if (config::patchFormCaching)
            PatchFormCaching();

        if (config::patchMaxStdio)
            PatchMaxStdio();

        if (config::patchRegularQuicksaves)
            PatchRegularQuicksaves();

        if (config::patchSaveAddedSoundCategories)
            PatchSaveAddedSoundCategories();

        if (config::patchScrollingDoesntSwitchPOV)
            PatchScrollingDoesntSwitchPOV();

        if (config::patchSleepWaitTime)
            PatchSleepWaitTime();

        if (config::patchTreeLODReferenceCaching && config::patchFormCaching)
            PatchTreeLODReferenceCaching();

        if (config::patchWaterflowAnimation)
            PatchWaterflowAnimation();

        if (config::experimentalMemoryManager)
            PatchMemoryManager();

        if (config::experimentalTreatAllModsAsMasters)
            PatchTreatAllModsAsMasters();

        return true;
    }
}

#include "patches.h"

namespace patches
{
    bool PatchAll()
    {
        if (config::patchFormCaching)
            PatchFormCaching();

        if (config::patchMaxStdio)
            PatchMaxStdio();

        if (config::patchRegularQuicksaves)
            PatchRegularQuicksaves();

        if (config::patchSaveAddedSoundCategories)
            PatchSaveAddedSoundCategories();

        if (config::patchTreeLODReferenceCaching && config::patchFormCaching)
            PatchTreeLODReferenceCaching();

        if (config::patchWaterflowAnimation)
            PatchWaterflowAnimation();

        if (config::experimentalMemoryManager)
            PatchMemoryManager();

        return true;
    }
}

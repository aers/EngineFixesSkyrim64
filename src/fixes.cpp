#include "fixes.h"


namespace fixes
{
    bool PatchAll()
    {
        if (config::fixDoublePerkApply)
            PatchDoublePerkApply();

        if (config::fixMemoryAccessErrors)
            PatchMemoryAccessErrors();

        if (config::fixMO5STypo)
            PatchMO5STypo();

        if (config::fixPerkFragmentIsRunning)
            PatchPerkFragmentIsRunning();

        if (config::fixRemovedSpellBook)
            PatchRemovedSpellBook();

        if (config::fixSlowTimeCameraMovement)
            PatchSlowTimeCameraMovement();

        if (config::fixTreeReflections)
            PatchTreeReflections();

        return true;
    }
}

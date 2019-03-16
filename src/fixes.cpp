#include "fixes.h"


namespace fixes
{
    bool PatchAll()
    {
        if (config::fixBethesdaNetCrash)
            PatchBethesdaNetCrash();

        if (config::fixBSLightingAmbientSpecular)
            PatchBSLightingAmbientSpecular();

        if (config::fixBSLightingShaderForceAlphaTest)
            PatchBSLightingShaderForceAlphaTest();

        if (config::fixDoublePerkApply)
            PatchDoublePerkApply();

        if (config::fixEquipShoutEventSpam)
            PatchEquipShoutEventSpam();

        if (config::fixGHeapLeakDetectionCrash)
            PatchGHeapLeakDetectionCrash();

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

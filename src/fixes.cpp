#include "fixes.h"


namespace fixes
{
    bool PatchAll()
    {
		if (config::fixArcheryDownwardAiming)
			PatchArcheryDownwardAiming();

        if (config::fixBethesdaNetCrash)
            PatchBethesdaNetCrash();

        if (config::fixBSLightingAmbientSpecular)
            PatchBSLightingAmbientSpecular();

        if (config::fixBSLightingShaderForceAlphaTest)
            PatchBSLightingShaderForceAlphaTest();

		if (config::fixConjurationEnchantAbsorbs)
			PatchConjurationEnchantAbsorbs();

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

        if (config::fixVerticalLookSensitivity)
            PatchVerticalLookSensitivity();

		if (config::fixAnimationLoadSignedCrash)
			PatchAnimationLoadSignedCrash();

        return true;
    }
}

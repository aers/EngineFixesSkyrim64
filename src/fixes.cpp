#include "fixes.h"

namespace fixes
{
    bool PatchAll()
    {
        if (config::fixAnimationLoadSignedCrash)
            PatchAnimationLoadSignedCrash();

        if (config::fixArcheryDownwardAiming)
            PatchArcheryDownwardAiming();

        if (config::fixBethesdaNetCrash)
            PatchBethesdaNetCrash();

        if (config::fixBSLightingAmbientSpecular)
            PatchBSLightingAmbientSpecular();

        if (config::fixBSLightingShaderForceAlphaTest)
            PatchBSLightingShaderForceAlphaTest();

        if (config::fixBSLightingShaderGeometryParallaxBug)
            PatchBSLightingShaderSetupGeometryParallax();

        if (config::fixBSTempEffectNiRTTI)
            PatchBSTempEffectNiRTTI();

        if (config::fixCalendarSkipping)
            PatchCalendarSkipping();

        if (config::fixCellInit)
            PatchCellInit();

        if (config::fixConjurationEnchantAbsorbs)
            PatchConjurationEnchantAbsorbs();

        if (config::fixDoublePerkApply)
            PatchDoublePerkApply();

        if (config::fixEquipShoutEventSpam)
            PatchEquipShoutEventSpam();

        if (config::fixGetKeywordItemCount)
            PatchGetKeywordItemCount();

        if (config::fixGHeapLeakDetectionCrash)
            PatchGHeapLeakDetectionCrash();

        if (config::fixLipSync)
            PatchLipSync();

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

        if (config::fixUnequipAllCrash)
            PatchUnequipAllCrash();

        if (config::fixVerticalLookSensitivity)
            PatchVerticalLookSensitivity();

        if (config::fixWeaponBlockScaling)
            PatchWeaponBlockScaling();

        return true;
    }
}

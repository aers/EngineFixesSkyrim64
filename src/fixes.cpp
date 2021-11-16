#include "fixes.h"

namespace fixes
{
    bool PatchAll()
    {
        if (*config::fixAnimationLoadSignedCrash)
            PatchAnimationLoadSignedCrash();

        if (*config::fixArcheryDownwardAiming)
            PatchArcheryDownwardAiming();

        if (*config::fixBethesdaNetCrash)
            PatchBethesdaNetCrash();

        if (*config::fixBSLightingAmbientSpecular)
            PatchBSLightingAmbientSpecular();

        if (*config::fixBSLightingShaderForceAlphaTest)
            PatchBSLightingShaderForceAlphaTest();

        if (*config::fixBSLightingShaderGeometryParallaxBug)
            PatchBSLightingShaderSetupGeometryParallax();

        if (*config::fixBSTempEffectNiRTTI)
            PatchBSTempEffectNiRTTI();

        if (*config::fixCalendarSkipping)
            PatchCalendarSkipping();

        if (*config::fixCellInit)
            PatchCellInit();

        if (*config::fixCreateArmorNodeNullptrCrash)
            PatchCreateArmorNodeNullptrCrash();

        if (*config::fixConjurationEnchantAbsorbs)
            PatchConjurationEnchantAbsorbs();

        if (*config::fixDoublePerkApply)
            PatchDoublePerkApply();

        if (*config::fixEquipShoutEventSpam)
            PatchEquipShoutEventSpam();

        if (*config::fixFaceGenMorphDataHeadNullptrCrash)
            PatchFaceGenMorphDataHeadNullptrCrash();

        if (*config::fixGetKeywordItemCount)
            PatchGetKeywordItemCount();

        if (*config::fixGHeapLeakDetectionCrash)
            PatchGHeapLeakDetectionCrash();

        if (*config::fixGlobalTime)
            PatchGlobalTime();

        if (*config::fixInitializeHitDataNullptrCrash)
            PatchInitializeHitDataNullptrCrash();

        if (*config::fixLipSync)
            PatchLipSync();

        if (*config::fixMemoryAccessErrors)
            PatchMemoryAccessErrors();

        if (*config::fixMusicOverlap)
            PatchMusicOverlap();

        if (*config::fixMO5STypo)
            PatchMO5STypo();

        if (*config::fixNullProcessCrash)
            PatchNullProcessCrash();

        if (*config::fixPerkFragmentIsRunning)
            PatchPerkFragmentIsRunning();

        if (*config::fixRemovedSpellBook)
            PatchRemovedSpellBook();

        if (*config::fixShadowSceneNodeNullptrCrash)
            PatchShadowSceneNodeNullptrCrash();

        if (*config::fixSlowTimeCameraMovement)
            PatchSlowTimeCameraMovement();

        if (*config::fixTorchLandscape)
            PatchTorchLandscape();

        if (*config::fixVerticalLookSensitivity)
            PatchVerticalLookSensitivity();

        if (*config::fixWeaponBlockScaling)
            PatchWeaponBlockScaling();

        return true;
    }
}

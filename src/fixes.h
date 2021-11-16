#pragma once

#include "config.h"
#include "offsets.h"

namespace fixes
{
    bool PatchAnimationLoadSignedCrash();
    bool PatchArcheryDownwardAiming();
    bool PatchBethesdaNetCrash();
    bool PatchBSLightingAmbientSpecular();
    bool PatchBSLightingShaderForceAlphaTest();
    bool PatchBSLightingShaderSetupGeometryParallax();
    bool PatchBSTempEffectNiRTTI();
    bool PatchCalendarSkipping();
    bool PatchCellInit();
    bool PatchConjurationEnchantAbsorbs();
    bool PatchCreateArmorNodeNullptrCrash();
    bool PatchDoublePerkApply();
    bool PatchEquipShoutEventSpam();
    bool PatchFaceGenMorphDataHeadNullptrCrash();
    bool PatchGetKeywordItemCount();
    bool PatchGHeapLeakDetectionCrash();
    bool PatchGlobalTime();
    bool PatchInitializeHitDataNullptrCrash();
    bool PatchLipSync();
    bool PatchMemoryAccessErrors();
    bool PatchMusicOverlap();
    bool PatchMO5STypo();
    bool PatchNullProcessCrash();
    bool PatchPerkFragmentIsRunning();
    bool PatchRemovedSpellBook();
    bool PatchSaveScreenshots();
    bool PatchShadowSceneNodeNullptrCrash();
    bool PatchSlowTimeCameraMovement();
    bool PatchTorchLandscape();
    bool PatchTreeReflections();
    bool PatchVerticalLookSensitivity();
    bool PatchWeaponBlockScaling();

    bool PatchAll();
}

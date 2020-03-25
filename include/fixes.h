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
    bool PatchDoublePerkApply();
    bool PatchEquipShoutEventSpam();
    bool PatchGetKeywordItemCount();
    bool PatchGHeapLeakDetectionCrash();
    bool PatchLipSync();
    bool PatchMemoryAccessErrors();
    bool PatchMO5STypo();
    bool PatchPerkFragmentIsRunning();
    bool PatchRemovedSpellBook();
    bool PatchSaveScreenshots();
    bool PatchSlowTimeCameraMovement();
    bool PatchTreeReflections();
    bool PatchUnequipAllCrash();
    bool PatchVerticalLookSensitivity();
    bool PatchWeaponBlockScaling();

    bool PatchAll();
}

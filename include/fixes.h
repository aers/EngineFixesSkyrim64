#pragma once

#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/Relocation.h"
#include "skse64_common/SafeWrite.h"

#include "xbyak/xbyak.h"

#include "config.h"
#include "offsets.h"

namespace fixes
{
    bool PatchBethesdaNetCrash();
    bool PatchBSLightingAmbientSpecular();
    bool PatchBSLightingShaderForceAlphaTest();
	bool PatchConjurationEnchantAbsorbs();
    bool PatchDoublePerkApply();
    bool PatchEquipShoutEventSpam();
    bool PatchGHeapLeakDetectionCrash();
    bool PatchMemoryAccessErrors();
    bool PatchMO5STypo();
    bool PatchPerkFragmentIsRunning();
    bool PatchRemovedSpellBook();
    bool PatchSaveScreenshots();
    bool PatchSlowTimeCameraMovement();
    bool PatchTreeReflections();
    bool PatchVerticalLookSensitivity();

    bool PatchAll();
}
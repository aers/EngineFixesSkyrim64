#include "fixes.h"

#include "animation_load_signed_crash.h"
#include "archery_downward_aiming.h"
#include "bethesda_net_crash.h"
#include "bslightingambientspecular.h"
#include "bslightingshader_force_alpha_test.h"
#include "bslightingshader_parallax_bug.h"
#include "bstempeffect_nirtti.h"
#include "calendar_skipping.h"
#include "cell_init.h"
#include "climate_load.h"
#include "conjuration_enchant_absorbs.h"
#include "create_armor_node_nullptr_crash.h"
#include "double_perk_apply.h"
#include "memory_access_errors.h"

namespace Fixes
{
    void Load()
    {
        if (Settings::Fixes::bArcheryDownwardAiming)
            ArcheryDownwardAiming::Install();

        if (Settings::Fixes::bAnimationLoadSignedCrash)
            AnimationLoadSignedCrash::Install();

        if (Settings::Fixes::bBethesdaNetCrash)
            BethesdaNetCrash::Install();

        if (Settings::Fixes::bBSLightingAmbientSpecular)
            BSLightingAmbientSpecular::Install();

        if (Settings::Fixes::bBSLightingShaderForceAlphaTest)
            BSLightingShaderForceAlphaTest::Install();

        if (Settings::Fixes::bBSLightingShaderParallaxBug)
            BSLightingShaderParallaxBug::Install();

        if (Settings::Fixes::bBSTempEffectNiRTTI)
            BSTempEffectNiRTTI::Install();

        if (Settings::Fixes::bCalendarSkipping)
            CalendarSkipping::Install();

        if (Settings::Fixes::bCellInit)
            CellInit::Install();

        if (Settings::Fixes::bClimateLoad)
            ClimateLoad::Install();

        if (Settings::Fixes::bConjurationEnchantAbsorbs)
            ConjurationEnchantAbsorbs::Install();

        if (Settings::Fixes::bCreateArmorNodeNullPtrCrash)
            CreateArmorNodeNullPtrCrash::Install();

        if (Settings::Fixes::bDoublePerkApply)
            DoublePerkApply::Install();

        if (Settings::Fixes::bMemoryAccessErrors)
            MemoryAccessErrors::Install();
    }
}
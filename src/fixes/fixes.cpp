#include "fixes.h"

#include "animation_load_signed_crash.h"
#include "archery_downward_aiming.h"
#include "bethesda_net_crash.h"
#include "bslightingambientspecular.h"
#include "bslightingshader_force_alpha_test.h"
#include "bslightingshader_parallax_bug.h"
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

        if (Settings::Fixes::bMemoryAccessErrors)
            MemoryAccessErrors::Install();
    }
}
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
#include "equip_shout_event_spam.h"
#include "facegen_morphdatahead_nullptr_crash.h"
#include "get_keyword_item_count.h"
#include "gheap_leak_detection_crash.h"
#include "global_time.h"
#include "initialize_hit_data_nullptr_crash.h"
#include "lip_sync.h"
#include "memory_access_errors.h"
#include "mo5s_typo.h"
#include "music_overlap.h"
#include "null_process_crash.h"
#include "perk_fragment_is_running.h"
#include "removed_spellbook.h"
#include "shadowscenenode_nullptr_crash.h"
#include "torch_landscape.h"
#include "tree_reflections.h"
#include "vertical_look_sensitivity.h"
#include "weapon_block_scaling.h"

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

        if (Settings::Fixes::bEquipShoutEventSpam)
            EquipShoutEventSpam::Install();

        if (Settings::Fixes::bFaceGenMorphDataHeadNullPtrCrash)
            FaceGenMorphDataHeadNullPtrCrash::Install();

        if (Settings::Fixes::bGetKeywordItemCount)
            GetKeywordItemCount::Install();

        if (Settings::Fixes::bGHeapLeakDetectionCrash)
            GHeapLeakDetectionCrash::Install();

        if (Settings::Fixes::bGlobalTime)
            GlobalTime::Install();

        if (Settings::Fixes::bInitializeHitDataNullPtrCrash)
            InitializeHitDataNullPtrCrash::Install();

        if (Settings::Fixes::bLipSync)
            LipSync::Install();

        if (Settings::Fixes::bMemoryAccessErrors)
            MemoryAccessErrors::Install();

        if (Settings::Fixes::bMO5STypo)
            MO5STypo::Install();

        if (Settings::Fixes::bMusicOverlap)
            MusicOverlap::Install();

        if (Settings::Fixes::bNullProcessCrash)
            NullProcessCrash::Install();

        if (Settings::Fixes::bPerkFragmentIsRunning)
            PerkFragmentIsRunning::Install();

        if (Settings::Fixes::bRemovedSpellBook)
            RemovedSpellBook::Install();

        if (Settings::Fixes::bShadowSceneNodeNullPtrCrash)
            ShadowSceneNodeNullPtrCrash::Install();

        if (Settings::Fixes::bTorchLandscape)
            TorchLandscape::Install();

        if (Settings::Fixes::bTreeReflections)
            TreeReflections::Install();

        if (Settings::Fixes::bVerticalLookSensitivity)
            VerticalLookSensitivity::Install();

        if (Settings::Fixes::bWeaponBlockScaling)
            WeaponBlockScaling::Install();
    }
}
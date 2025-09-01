#include "fixes.h"

#include "animation_load_signed_crash.h"
#include "archery_downward_aiming.h"
#include "bethesda_net_crash.h"
#include "bgskeywordform_load_crash.h"
#include "bslightingambientspecular.h"
#include "bslightingshader_force_alpha_test.h"
#include "bslightingshader_parallax_bug.h"
#include "bslightingshaderproperty_shadowmap.h"
#include "bstempeffect_nirtti.h"
#include "calendar_skipping.h"
#include "cell_init.h"
#include "climate_load.h"
#include "conjuration_enchant_absorbs.h"
#include "create_armor_node_nullptr_crash.h"
#include "double_perk_apply.h"
#include "equip_shout_event_spam.h"
#include "esl_cell_loading_bug.h"
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
#include "vertical_look_sensitivity.h"
#include "weapon_block_scaling.h"

namespace Fixes
{
    void Load()
    {
        if (Settings::Fixes::bArcheryDownwardAiming.GetValue())
            ArcheryDownwardAiming::Install();

        if (Settings::Fixes::bAnimationLoadSignedCrash.GetValue())
            AnimationLoadSignedCrash::Install();

        if (Settings::Fixes::bBethesdaNetCrash.GetValue())
            BethesdaNetCrash::Install();

        if (Settings::Fixes::bBGSKeywordFormLoadCrash.GetValue())
            BGSKeywordFormLoadCrash::Install();

        if (Settings::Fixes::bBSLightingAmbientSpecular.GetValue())
            BSLightingAmbientSpecular::Install();

        if (Settings::Fixes::bBSLightingShaderForceAlphaTest.GetValue())
            BSLightingShaderForceAlphaTest::Install();

        if (Settings::Fixes::bBSLightingShaderParallaxBug.GetValue())
            BSLightingShaderParallaxBug::Install();

        if (Settings::Fixes::bBSLightingShaderPropertyShadowMap.GetValue())
            BSLightingShaderPropertyShadowMap::Install();

        if (Settings::Fixes::bBSTempEffectNiRTTI.GetValue())
            BSTempEffectNiRTTI::Install();

        if (Settings::Fixes::bCalendarSkipping.GetValue())
            CalendarSkipping::Install();

        if (Settings::Fixes::bCellInit.GetValue())
            CellInit::Install();

        if (Settings::Fixes::bClimateLoad.GetValue())
            ClimateLoad::Install();

        if (Settings::Fixes::bConjurationEnchantAbsorbs.GetValue())
            ConjurationEnchantAbsorbs::Install();

        if (Settings::Fixes::bCreateArmorNodeNullPtrCrash.GetValue())
            CreateArmorNodeNullPtrCrash::Install();

        if (Settings::Fixes::bDoublePerkApply.GetValue())
            DoublePerkApply::Install();

        if (Settings::Fixes::bEquipShoutEventSpam.GetValue())
            EquipShoutEventSpam::Install();

        if (Settings::Fixes::bESLCELLLoadBug.GetValue())
            ESLCELLLoadingBugs::Install();

        if (Settings::Fixes::bFaceGenMorphDataHeadNullPtrCrash.GetValue())
            FaceGenMorphDataHeadNullPtrCrash::Install();

        if (Settings::Fixes::bGetKeywordItemCount.GetValue())
            GetKeywordItemCount::Install();

        if (Settings::Fixes::bGHeapLeakDetectionCrash.GetValue())
            GHeapLeakDetectionCrash::Install();

        if (Settings::Fixes::bGlobalTime.GetValue())
            GlobalTime::Install();

        if (Settings::Fixes::bInitializeHitDataNullPtrCrash.GetValue())
            InitializeHitDataNullPtrCrash::Install();

        if (Settings::Fixes::bLipSync.GetValue())
            LipSync::Install();

        if (Settings::Fixes::bMemoryAccessErrors.GetValue())
            MemoryAccessErrors::Install();

        if (Settings::Fixes::bMO5STypo.GetValue())
            MO5STypo::Install();

        if (Settings::Fixes::bMusicOverlap.GetValue())
            MusicOverlap::Install();

        if (Settings::Fixes::bNullProcessCrash.GetValue())
            NullProcessCrash::Install();

        if (Settings::Fixes::bPerkFragmentIsRunning.GetValue())
            PerkFragmentIsRunning::Install();

        if (Settings::Fixes::bRemovedSpellBook.GetValue())
            RemovedSpellBook::Install();

        if (Settings::Fixes::bShadowSceneNodeNullPtrCrash.GetValue())
            ShadowSceneNodeNullPtrCrash::Install();

        if (Settings::Fixes::bTorchLandscape.GetValue())
            TorchLandscape::Install();

        if (Settings::Fixes::bVerticalLookSensitivity.GetValue())
            VerticalLookSensitivity::Install();

        if (Settings::Fixes::bWeaponBlockScaling.GetValue())
            WeaponBlockScaling::Install();
    }
}
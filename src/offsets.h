#pragma once

namespace offsets
{
    namespace Common
    {
        constexpr REL::ID g_SecondsSinceLastFrame_WorldTime(410199);
        constexpr REL::ID g_SecondsSinceLastFrame_RealTime(410200);
    }

    // patches
    namespace AchievementsWithMods
    {
        // 48 83 EC 28 C6 44 24 ? ?
        constexpr REL::ID AchievementModsEnabledFunction(441528);
    }

    namespace DisableChargenPrecache
    {
        constexpr std::array todo = {
            REL::ID(52369), // PreCache - 48 83 EC 48 48 8B 0D ? ? ? ? 48 85 C9
            REL::ID(52381) // PreCacheClear - E8 ? ? ? ? E8 ? ? ? ? C6 87 ? ? ? ? ?
         };
    }

    namespace FormCaching
    {
        // E8 ? ? ? ? 48 8B DD
        constexpr REL::ID LookupFormByID(14617);
        constexpr REL::ID GlobaFormTable(400507);
        constexpr REL::ID GlobalFormTableLock(400517);

        // E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 08 48 8B D3
        constexpr REL::ID UnkFormFunc0(14627);
        // E8 ? ? ? ? 90 48 8B CD E8 ? ? ? ? 90 EB 07
        constexpr REL::ID UnkFormFunc1(14688);
        // E8 ? ? ? ? 40 B5 01 49 8B CD
        constexpr REL::ID UnkFormFunc2(14710);
    }

    namespace MemoryManager
    {
        // E8 ? ? ? ? 48 89 6E 68
        constexpr REL::ID AutoScrapBuffer_Ctor(68108);
        // E8 ? ? ? ? 8D 53 18
        constexpr REL::ID AutoScrapBuffer_Dtor(68109);

        // E8 ? ? ? ? 0F B6 53 20
        constexpr REL::ID MemoryManager_Allocate(68115);
        constexpr REL::ID MemoryManager_ReAllocate(68116);
        constexpr REL::ID MemoryManager_DeAllocate(68117);

        // E8 ? ? ? ? 48 63 FF
        constexpr REL::ID MemoryManager_Init(68121);

        constexpr REL::ID ScrapHeap_vtbl(236607);

        constexpr REL::ID ScrapHeap_ctor(68142);
        // E8 ? ? ? ? 44 0F B7 F5
        constexpr REL::ID ScrapHeap_Allocate(68144);
        // E8 ? ? ? ? 45 89 77 0C
        constexpr REL::ID ScrapHeap_DeAllocate(68146);
        // 40 53 48 83 EC 20 83 79 78 00
        constexpr REL::ID ScrapHeap_Clean(68152);
        // FF 49 78
        constexpr REL::ID ScrapHeap_ClearKeepPages(68151);
        // E8 ? ? ? ? 49 8B 44 24 ? 48 3B F0
        constexpr REL::ID ScrapHeap_InsertFreeBlock(68155);
        // E8 ? ? ? ? 48 8B 06 49 23 C7
        constexpr REL::ID ScrapHeap_RemoveFreeBlock(68156);
        // FF 41 78
        constexpr REL::ID ScrapHeap_SetKeepPages(68150);
        // vtable ref
        constexpr REL::ID ScrapHeap_Dtor(68143);
    }

    namespace RegularQuicksaves
    {
        constexpr REL::ID QuickSaveLoadHandler_ProcessButton(52251); // vf5
    }

    namespace SafeExit
    {
        constexpr REL::ID WinMain(36544);
    }

    namespace SaveAddedSoundCategories
    {
        constexpr REL::ID INIPrefSettingCollection_vtbl(187227);
        constexpr REL::ID BGSSoundCategory_BSISoundCategory_vtbl(191093);
    }

    namespace SaveGameMaxSize
    {
        constexpr REL::ID Win32FileType_CopyToBuffer(109378); // vf18
        constexpr REL::ID Win32FileType_Ctor(109355); // vtbl cref
        // E8 ? ? ? ? 8B D0 4C 8B CB
        constexpr REL::ID ScrapHeap_GetMaxSize(36095);
    }

    namespace ScaleFormAllocator
    {
        // E8 ? ? ? ? 48 89 05 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 83 3D ? ? ? ? ? 75 13
        constexpr REL::ID ScaleFormManager_Init(82323);
    }

    namespace ScrollingDoesntSwitchPOV
    {
        constexpr REL::ID FirstPersonState_PlayerInputHandler_ProcessButton(50730); // vf4
        constexpr REL::ID ThirdPersonState_PlayerInputHandler_ProcessButton(50906);
    }

    namespace SleepWaitTime
    {
        constexpr REL::ID SleepWaitMenu_vf4(52486);
    }

    namespace TreeLodReferenceCaching
    {
        // 4C 8B DC 41 55 48 83 EC 70
        constexpr REL::ID UpdateBlockVisibility(31660);
        // E8 ? ? ? ? 66 89 04 37
        constexpr REL::ID Float2Half(76217);
    }

    namespace WaterflowAnimation
    {
        // 48 8B C4 55 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 C7 84 24 ? ? ? ? ? ? ? ? 48 89 58 08 48 89 70 10 48 89 78 18 48 8D 6C 24 ? 48 83 E5 C0 4C 8B E1
        constexpr REL::ID Main_Update(36564);
        constexpr REL::ID g_ApplicationRunTime(410201);

        constexpr REL::ID WaterShader_SetupMaterial(107363); // BSWaterShader_vf4
    }

    // fixes
    namespace AnimationLoadSignedCrash
    {
        // E8 ? ? ? ? 90 48 83 C4 58 C3 32 C0
        constexpr REL::ID Movsx(65232);
    }

    namespace ArcheryDownwardAiming
    {
        // E8 ? ? ? ? 8B 83 ? ? ? ? C1 E8 12 A8 01 74 3F
        constexpr REL::ID MoveFunctionCall(44027);
    }

    namespace BSLightingAmbientSpecular
    {
        constexpr REL::ID BSLightingShader_SetupMaterial_AmbientSpecular(107298);  // BSLightingShader_vf4
        constexpr REL::ID g_AmbientSpecularAndFresnel(390997); // referenced at above location
        constexpr REL::ID BSLightingShader_SetupGeometry_AddAmbientSpecular(107300); // BSLightingShader_vf6
    }

    namespace CalendarSkipping
    {
        // these are all the callsites for function at 40 53 48 81 EC ? ? ? ? 48 8B 41 30
        constexpr std::array todo = {
            std::make_pair(13328, 0xE2),
            std::make_pair(36564, 0x266),
            std::make_pair(36566, 0x3A),
            std::make_pair(40445, 0x282),
            std::make_pair(40485, 0x78)
        };
    }

    namespace CellInit
    {
        // E8 ? ? ? ? EB 37 48 8B 07
        constexpr REL::ID TESObjectCELL_GetLocation_ExtraDataList_GetLocation_Call(18905);
    }

    namespace ClimateLoad
    {
        const auto todo = std::array{
            std::make_pair(26218, 0x1A0),
            std::make_pair(35642, 0x241)
        };

        constexpr REL::ID SetCurrentClimate(26239);
    }

    namespace ConjurationEnchantAbsorbs
    {
        constexpr REL::ID EnchantmentItem_vtbl(186389);
    }

    namespace CreateArmorNodeNullPtr
    {
        // E8 ? ? ? ? 48 8B F0 41 FF C6
        constexpr REL::ID SubFunction_PatchLocation(15712);  // mov     esi, edi
    }

    namespace DoublePerkApply
    {
        constexpr REL::ID BGSPerk_ApplyPerk(23822);
        constexpr REL::ID Actor_RemoveBasePerks(37704);
        constexpr REL::ID Actor_vtbl(207511);
        constexpr REL::ID Character_vtbl(207886);
        constexpr REL::ID PlayerCharacter_vtbl(208040);
    }

    namespace EquipShoutEventSpam
    {
        // E8 ? ? ? ? 4C 8B 4C 24 ? 44 8B C7
        constexpr REL::ID FuncBase(38770);
    }

    namespace FaceGenMorphDataHeadNullptr
    {
        // 48 8B C4 57 41 56 41 57 48 83 EC 70 48 C7 40 ? ? ? ? ? 48 89 58 08 48 89 68 10 48 89 70 18 0F 29 70 D8 0F 28 F2
        constexpr REL::ID FuncBase(26918);
    }

    namespace GHeapLeakDetectionCrash
    {
        // E8 ? ? ? ? 48 8B 07 33 D2 48 8B CF FF 10
        constexpr REL::ID FuncBase(87837);
    }

    namespace GlobalTime
    {
        constexpr std::array todo = {
            std::make_pair(51049, 0xB70), // BookMenu::vf4
            std::make_pair(52486, 0x1BE) // SleepWaitMenu::vf4
        };
    }

    namespace InitializeHitDataNullptr
    {
        // E8 ? ? ? ? 8B 9C 24 ? ? ? ? C1 EB 05
        constexpr REL::ID FuncBase(44001);
    }

    namespace LipSync
    {
        // 40 53 41 57 48 83 EC 38 44 8B 51 04
        constexpr REL::ID FuncBase(16267);
    }

    namespace MemoryAccessErrors
    {
        constexpr REL::ID BSLightingShaderMaterialSnow_vtbl(254759);
        constexpr REL::ID BSLightingShader_SetupMaterial(107298); // BSLightingShader::vf4

        constexpr REL::ID BGSShaderParticleGeometryData_vtbl(189948);

        constexpr REL::ID BSShadowDirectionalLight_vf16(108496);
    }

    namespace MO5STypo
    {
        // E8 ? ? ? ? E9 ? ? ? ? 81 F9 ? ? ? ? 7F 57
        constexpr REL::ID FuncBase(14827);
    }

    namespace MusicOverlap
    {
        constexpr REL::ID BGSMusicType_BSIMusicType_vtbl(191051); // BSIMusicType vtbl in BGSMusicType
    }

    namespace NullProcessCrash
    {
        // 48 83 EC 40 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 89 74 24 ? 48 8B DA 48 8B F1 48 85 D2
        constexpr REL::ID FuncBase1(38899);
        // E8 ? ? ? ? 41 0F B6 EE
        constexpr REL::ID FuncBase2(47338);
    }

    namespace PerkFragmentIsRunning
    {
        // 48 89 5C 24 ? 57 48 83 EC 20 33 FF 49 8B D9 49 89 39 48 85 C9 74 08 80 79 1A 3E 48 0F 44 F9 48 8B CF
        constexpr REL::ID FuncBase(21571); // ref "%s is running" string
    }

    namespace RemovedSpellBook
    {
        constexpr REL::ID TESObjectBOOK_vtbl(189577);
    }

    namespace SaveScreenshots
    {
        // E8 ? ? ? ? 33 C9 E8 ? ? ? ? 88 1D ? ? ? ?
        constexpr REL::ID BGSSaveLoadManager_ProcessEvents_RequestScreenshot(35772);
        // E8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8D 3D ? ? ? ?
        constexpr REL::ID MenuSave_RequestScreenshot(36555);
        constexpr REL::ID g_RequestSaveScreenshot(403755);
        // E8 ? ? ? ? 45 8B FD EB 20
        constexpr REL::ID ScreenshotRenderFunction(105674);
    }

    namespace ShaderFixes
    {
        // E8 ? ? ? ? 49 8B 8E ? ? ? ? 40 B6 01
        constexpr REL::ID BSBatchRenderer_SetupAndDrawPass(107644);
        constexpr REL::ID BSLightingShader_vtbl(255053);
        // 48 8B C4 44 89 40 18 48 89 50 10 48 89 48 08 55 53
        constexpr REL::ID BSLightingShader_SetupGeometry_ParallaxTechniqueLoc(107300);
    }

    namespace ShadowSceneNodeNullPtr
    {
        constexpr REL::ID FuncBase(106342);
    }

    namespace SlowTimeCameraMovement
    {
        constexpr std::array todo = {
            std::make_pair(50913, 0x3F), // 40 53 48 83 EC 70 F3 0F 10 51 ?
            std::make_pair(50913, 0xA1),
            std::make_pair(50913, 0x1BA),
            std::make_pair(50911, 0x268), // E8 ? ? ? ? 48 8B 43 38 48 85 C0 74 27
            std::make_pair(50921, 0x17) // E8 ? ? ? ? 48 8D 4B 4C
        };
    }

    namespace TorchLandscape
    {
        // E8 ? ? ? ? 48 8B 5C 24 ? 41 8B 46 18
        constexpr REL::ID AddLightCall(17610);
    }

    namespace TreeReflections
    {
        constexpr REL::ID BSDistantTreeShader_vf2_PatchLocation(107551);
    }

    namespace VerticalLookSensitivity
    {
        constexpr REL::ID ThirdPersonState_HandleLookInput(50914); // vf15
        constexpr REL::ID DragonCameraState_HandleLookInput(33119); // vf15
        constexpr REL::ID HorseCameraState_HandleLookInput(50770); // vf15
    }

    namespace WeaponBlockScaling
    {
        // E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 8B 03
        constexpr REL::ID FuncBase(44014);
    }

    // Warnings
    namespace DuplicateAddonNodeIndex
    {
        constexpr REL::ID BGSAddonNode_vtbl(188971);

        // E8 ? ? ? ? E9 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 10
        constexpr REL::ID Unk_DataReload_Func(36601); // DetectSignOut vtbl
    }

    namespace RefrHandleLimit
    {
        // 48 83 EC 38 33 D2 48 8D 0D ? ? ? ? (ctor)
        constexpr REL::ID g_RefrHandleArray(400622);
    }
}

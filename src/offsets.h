#pragma once

namespace offsets
{
    namespace Common
    {
        constexpr REL::Offset g_SecondsSinceLastFrame_WorldTime(0x30064C8);
        constexpr REL::Offset g_SecondsSinceLastFrame_RealTime(0x30064CC);
    }

    // patches
    namespace AchievementsWithMods
    {
        // 48 83 EC 28 C6 44 24 ? ?
        constexpr REL::Offset AchievementModsEnabledFunction(0x179A40);
    }

    namespace DisableChargenPrecache
    {
        constexpr std::array todo = {
            REL::Offset(0x8E2AA0), // PreCache - 48 83 EC 48 48 8B 0D ? ? ? ? 48 85 C9 
            REL::Offset(0x8E2D30) // PreCacheClear - E8 ? ? ? ? E8 ? ? ? ? C6 87 ? ? ? ? ? 
         };
    }

    namespace FormCaching
    {
        // E8 ? ? ? ? 48 8B DD
        constexpr REL::Offset LookupFormByID(0x19F110);
        constexpr REL::Offset GlobaFormTable(0x1F5E498);
        constexpr REL::Offset GlobalFormTableLock(0x1F5E928);

        // E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 C9 74 08 48 8B D3
        constexpr REL::Offset UnkFormFunc0(0x19F590);
        // E8 ? ? ? ? 90 48 8B CD E8 ? ? ? ? 90 EB 07
        constexpr REL::Offset UnkFormFunc1(0x1A0BE0);
        // E8 ? ? ? ? 40 B5 01 49 8B CD
        constexpr REL::Offset UnkFormFunc2(0x1A17E0);
    }

    namespace MemoryManager
    {
        // E8 ? ? ? ? 48 89 6E 68
        constexpr REL::Offset AutoScrapBuffer_Ctor(0xC26BF0);
        // E8 ? ? ? ? 8D 53 18
        constexpr REL::Offset AutoScrapBuffer_Dtor(0xC26C80);

        // E8 ? ? ? ? 0F B6 53 20
        constexpr REL::Offset MemoryManager_Allocate(0xC26F00);
        constexpr REL::Offset MemoryManager_ReAllocate(0xC27150);
        constexpr REL::Offset MemoryManager_DeAllocate(0xC27350);

        // E8 ? ? ? ? 48 63 FF 
        constexpr REL::Offset MemoryManager_Init(0xC274C0);

        constexpr REL::Offset ScrapHeap_vtbl(0x184C230);

        constexpr REL::Offset ScrapHeap_ctor(0xC28190);
        // E8 ? ? ? ? 44 0F B7 F5 
        constexpr REL::Offset ScrapHeap_Allocate(0xC28310);
        // E8 ? ? ? ? 45 89 77 0C
        constexpr REL::Offset ScrapHeap_DeAllocate(0xC28910);
        // 40 53 48 83 EC 20 83 79 78 00 
        constexpr REL::Offset ScrapHeap_Clean(0xC28AD0);
        // FF 49 78
        constexpr REL::Offset ScrapHeap_ClearKeepPages(0xC28AC0);
        // E8 ? ? ? ? 49 8B 44 24 ? 48 3B F0
        constexpr REL::Offset ScrapHeap_InsertFreeBlock(0xC28BF0);
        // E8 ? ? ? ? 48 8B 06 49 23 C7
        constexpr REL::Offset ScrapHeap_RemoveFreeBlock(0xC28CF0);
        // FF 41 78 
        constexpr REL::Offset ScrapHeap_SetKeepPages(0xC28AB0);
        // vtable ref
        constexpr REL::Offset ScrapHeap_Dtor(0xC282D0);
    }

    namespace RegularQuicksaves
    {
        constexpr REL::Offset QuickSaveLoadHandler_ProcessButton(0x8D9F60); // vf5
    }

    namespace SafeExit
    {
        constexpr REL::Offset WinMain(0x5D29F0);
    }

    namespace SaveAddedSoundCategories
    {
        constexpr REL::Offset INIPrefSettingCollection_vtbl(0x162A5D8);
        constexpr REL::Offset BGSSoundCategory_BSISoundCategory_vtbl(0x166E898);
    }
    
    namespace SaveGameMaxSize
    {
        constexpr REL::Offset Win32FileType_CopyToBuffer(0x146C530); // vf18
        constexpr REL::Offset Win32FileType_Ctor(0x146B1B0); // vtbl cref
        // E8 ? ? ? ? 8B D0 4C 8B CB
        constexpr REL::Offset ScrapHeap_GetMaxSize(0x5BCF70);
    }

    namespace ScaleFormAllocator
    {
        // E8 ? ? ? ? 48 89 05 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 83 3D ? ? ? ? ? 75 13
        constexpr REL::Offset ScaleFormManager_Init(0xF106D0);
    }

    namespace ScrollingDoesntSwitchPOV
    {
        constexpr REL::Offset FirstPersonState_PlayerInputHandler_ProcessButton(0x873650); // vf4
        constexpr REL::Offset ThirdPersonState_PlayerInputHandler_ProcessButton(0x87BD00);
    }

    namespace SleepWaitTime
    {
        constexpr REL::Offset SleepWaitMenu_vf4(0x8ECDB0);
    }

    namespace TreeLodReferenceCaching
    {
        // 4C 8B DC 41 55 48 83 EC 70
        constexpr REL::Offset UpdateBlockVisibility(0x4C2020);
        // E8 ? ? ? ? 66 89 04 37
        constexpr REL::Offset Float2Half(0xD7A7C0);
    }

    namespace WaterflowAnimation
    {
        // 48 8B C4 55 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 C7 84 24 ? ? ? ? ? ? ? ? 48 89 58 08 48 89 70 10 48 89 78 18 48 8D 6C 24 ? 48 83 E5 C0 4C 8B E1
        constexpr REL::Offset Main_Update(0x5D9F50);
        constexpr REL::Offset g_ApplicationRunTime(0x30064D0);

        constexpr REL::Offset WaterShader_SetupMaterial(0x141C9A0); // BSWaterShader_vf4
    }

    // fixes
    namespace AnimationLoadSignedCrash
    {
        // E8 ? ? ? ? 90 48 83 C4 58 C3 32 C0
        constexpr REL::Offset Movsx(0xB8B930 + 0xAA);
    }

    namespace ArcheryDownwardAiming
    {
        // E8 ? ? ? ? 8B 83 ? ? ? ? C1 E8 12 A8 01 74 3F 
        constexpr REL::Offset MoveFunctionCall(0x771400 + 0x434);
    }

    namespace BSLightingAmbientSpecular
    {
        constexpr REL::Offset BSLightingShader_SetupMaterial_AmbientSpecular(0x1417AF0 + 0x8CF);  // BSLightingShader_vf4
        constexpr REL::Offset g_AmbientSpecularAndFresnel(0x1EA2F1C); // referenced at above location
        constexpr REL::Offset BSLightingShader_SetupGeometry_AddAmbientSpecular(0x1418820 + 0x1271); // BSLightingShader_vf6
    }

    namespace CalendarSkipping
    {
        // these are all the callsites for function at 40 53 48 81 EC ? ? ? ? 48 8B 41 30
        constexpr std::array todo = {
            REL::Offset(0x15E820 + 0xE2),
            REL::Offset(0x5D9F50 + 0x266),
            REL::Offset(0x5DAC80 + 0x3A),
            REL::Offset(0x6C5840 + 0x282),
            REL::Offset(0x6CA780 + 0x78)
        };
    }

    namespace CellInit
    {
        // E8 ? ? ? ? EB 37 48 8B 07
        constexpr REL::Offset TESObjectCELL_GetLocation_ExtraDataList_GetLocation_Call(0x27EAB0 + 0x114);
    }

    namespace ConjurationEnchantAbsorbs
    {
        constexpr REL::Offset EnchantmentItem_vtbl(0x161B920);
    }

    namespace CreateArmorNodeNullPtr
    {
        // E8 ? ? ? ? 48 8B F0 41 FF C6
        constexpr REL::Offset SubFunction_PatchLocation(0x1D6740 + 0x51B);  // mov     esi, edi
    }

    namespace DoublePerkApply
    {
        // 48 85 D2 74 7C 48 89 5C 24 ?
        constexpr REL::Offset QueueApplyPerk(0x5EA9E0);
        // E8 ? ? ? ? 48 8B 03 48 8B CB FF 90 ? ? ? ? 80 3D ? ? ? ? ?
        constexpr REL::Offset Handle_Add_Rf(0x6A9860);
        // 40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 C7 85 ? ? ? ? ? ? ? ? 0F 29 B4 24 ? ? ? ?
        constexpr REL::Offset BSTaskPool_HandleTask_Movzx(0x5EAE40 + 0x2164);
        // 48 85 D2 74 7C 48 89 5C 24 ?
        constexpr std::uintptr_t Unknown_Add_Function = 0x5EA9E0;
        constexpr REL::Offset Unknown_Add_Function_Movzx1(Unknown_Add_Function + 0x1A);
        constexpr REL::Offset Unknown_Add_Function_Movzx2(Unknown_Add_Function + 0x46);
        // 48 83 EC 38 48 83 79 ? ? 74 2B
        constexpr REL::Offset Next_Formid_Get_Hook(0x6A9810 + 0x1B);
        // 48 89 6C 24 ? 57 41 56 41 57 48 83 EC 40 8B 41 70
        constexpr REL::Offset Do_Handle_Hook(0x34EAC0 + 0x11);
        // 48 83 EC 38 48 85 D2 74 23
        constexpr REL::Offset Do_Add_Hook(0x34EF30 + 0x11);
    }

    namespace EquipShoutEventSpam
    {
        // E8 ? ? ? ? 4C 8B 4C 24 ? 44 8B C7
        constexpr REL::Offset FuncBase(0x6582C0);
    }

    namespace FaceGenMorphDataHeadNullptr
    {
        // 48 8B C4 57 41 56 41 57 48 83 EC 70 48 C7 40 ? ? ? ? ? 48 89 58 08 48 89 68 10 48 89 70 18 0F 29 70 D8 0F 28 F2
        constexpr REL::Offset FuncBase(0x3EE7D0);
    }

    namespace GHeapLeakDetectionCrash
    {
        // E8 ? ? ? ? 48 8B 07 33 D2 48 8B CF FF 10
        constexpr REL::Offset FuncBase(0x104B480);
    }

    namespace GlobalTime
    {
        constexpr std::array todo = {
            REL::Offset(0x8843C0 + 0xB70), // BookMenu::vf4
            REL::Offset(0x8ECDB0 + 0x1BE) // SleepWaitMenu::vf4
        };
    }

    namespace InitializeHitDataNullptr
    {
        // E8 ? ? ? ? 8B 9C 24 ? ? ? ? C1 EB 05
        constexpr REL::Offset FuncBase(0x76EDB0);
    }

    namespace LipSync
    {
        // 40 53 41 57 48 83 EC 38 44 8B 51 04
        constexpr REL::Offset FuncBase(0x1FCDB0);
    }

    namespace MemoryAccessErrors
    {
        constexpr REL::Offset BSLightingShaderMaterialSnow_vtbl(0x1968EC0);
        constexpr REL::Offset BSLightingShader_SetupMaterial(0x1417AF0); // BSLightingShader::vf4

        constexpr REL::Offset BGSShaderParticleGeometryData_vtbl(0x1657868);

        constexpr REL::Offset BSShadowDirectionalLight_vf16(0x144D9F0);
    }

    namespace MO5STypo
    {
        // E8 ? ? ? ? E9 ? ? ? ? 81 F9 ? ? ? ? 7F 57
        constexpr REL::Offset FuncBase(0x1A6750);
    }

    namespace MusicOverlap
    {
        constexpr REL::Offset BGSMusicType_BSIMusicType_DoFinish(0x2DFC30); // BSIMusicType vtbl in BGSMusicType vf3
    }
    
    namespace NullProcessCrash
    {
        // 48 83 EC 40 48 C7 44 24 ? ? ? ? ? 48 89 5C 24 ? 48 89 74 24 ? 48 8B DA 48 8B F1 48 85 D2
        constexpr REL::Offset FuncBase1(0x65DCA0);
        // E8 ? ? ? ? 41 0F B6 EE 
        constexpr REL::Offset FuncBase2(0x7EA1C0);
    }
    
    namespace PerkFragmentIsRunning
    {
        // 48 89 5C 24 ? 57 48 83 EC 20 33 FF 49 8B D9 49 89 39 48 85 C9 74 08 80 79 1A 3E 48 0F 44 F9 48 8B CF
        constexpr REL::Offset FuncBase(0x2EF830); // ref "%s is running" string
    }

    namespace RemovedSpellBook
    {
        constexpr REL::Offset TESObjectBOOK_vtbl(0x1650998);
    }

    namespace SaveScreenshots
    {
        // E8 ? ? ? ? 33 C9 E8 ? ? ? ? 88 1D ? ? ? ?
        constexpr REL::Offset BGSSaveLoadManager_ProcessEvents_RequestScreenshot(0x5AAD20 + 0x1C6);
        // E8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8D 3D ? ? ? ? 
        constexpr REL::Offset MenuSave_RequestScreenshot(0x5D7CB0 + 0x5D0);
        constexpr REL::Offset g_RequestSaveScreenshot(0x2FD3640);
        // E8 ? ? ? ? 45 8B FD EB 20
        constexpr std::uintptr_t ScreenshotRenderFunction = 0x13BD6A0;
        constexpr REL::Offset ScreenshotRenderFunction_Jnz(ScreenshotRenderFunction + 0x17D);
        constexpr REL::Offset ScreenshotRenderFunction_RenderTargetHook1(ScreenshotRenderFunction + 0x294);
        constexpr REL::Offset ScreenshotRenderFunction_RenderTargetHook2(ScreenshotRenderFunction + 0x307);
        constexpr REL::Offset ScreenshotRenderFunction_OrigJnz(ScreenshotRenderFunction + 0x3B1);
    }

    namespace ShaderFixes
    {
        // E8 ? ? ? ? 49 8B 8E ? ? ? ? 40 B6 01
        constexpr REL::Offset BSBatchRenderer_SetupAndDrawPass(0x142F580);
        constexpr REL::Offset BSLightingShader_vtbl(0x196FFA0);
        // 48 8B C4 44 89 40 18 48 89 50 10 48 89 48 08 55 53 
        constexpr REL::Offset BSLightingShader_SetupGeometry_ParallaxTechniqueLoc(0x1418820 + 0xB5D);
    }

    namespace ShadowSceneNodeNullPtr
    {
        constexpr REL::Offset FuncBase(0x13DDD20);
    }

    namespace SlowTimeCameraMovement
    {
        // 40 53 48 83 EC 70 F3 0F 10 51 ? 
        constexpr std::uintptr_t ThirdPersonState_UpdateDelayedParameters = 0x87C810;

        constexpr std::array todo = {
            REL::Offset(ThirdPersonState_UpdateDelayedParameters + 0x2F),
            REL::Offset(ThirdPersonState_UpdateDelayedParameters + 0xA1),
            REL::Offset(ThirdPersonState_UpdateDelayedParameters + 0x1BA),
            REL::Offset(0x87C3A0 + 0x268), // E8 ? ? ? ? 48 8B 43 38 48 85 C0 74 27
            REL::Offset(0x87CE50 + 0x17) // E8 ? ? ? ? 48 8D 4B 4C
        };
    }

    namespace TorchLandscape
    {
        // E8 ? ? ? ? 48 8B 5C 24 ? 41 8B 46 18
        constexpr REL::Offset AddLightCall(0x22D490 + 0x530);
    }

    namespace TreeReflections
    {
        constexpr REL::Offset BSDistantTreeShader_vf2_PatchLocation(0x1428220 + 0x35);
    }

    namespace VerticalLookSensitivity
    {
        constexpr REL::Offset ThirdPersonState_HandleLookInput(0x87CAE0 + 0x65); // vf15
        constexpr REL::Offset DragonCameraState_HandleLookInput(0x512D80 + 0x53); // vf15
        constexpr REL::Offset HorseCameraState_HandleLookInput(0x8752C0 + 0x53); // vf15
    }

    namespace WeaponBlockScaling
    {
        // E8 ? ? ? ? 48 8B CF E8 ? ? ? ? 8B 03
        constexpr REL::Offset FuncBase(0x76FAE0);
    }

    // Warnings
    namespace DuplicateAddonNodeIndex
    {
        constexpr REL::Offset BGSAddonNode_vtbl(0x16444F0);

        // E8 ? ? ? ? E9 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 10
        constexpr REL::Offset Unk_DataReload_Func(0x5DCD70); // DetectSignOut vtbl
    }

    namespace RefrHandleLimit
    {
        // 48 83 EC 38 33 D2 48 8D 0D ? ? ? ? (ctor)
        constexpr REL::Offset g_RefrHandleArray(0x1F5EF10);
    }
}

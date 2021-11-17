#pragma once

namespace offsets
{
    namespace Common
    {
        constexpr REL::Offset g_SecondsSinceLastFrame_WorldTime(0x30064C8);
        constexpr REL::Offset g_SecondsSinceLastFrame_RealTime(0x30064CC);
    }

    // fixes
    namespace DoublePerkApply
    {
        // 48 85 D2 74 7C 48 89 5C 24 ?
        constexpr REL::Offset QueueApplyPerk(0x5EA9E0);
        // E8 ? ? ? ? 48 8B 03 48 8B CB FF 90 ? ? ? ? 80 3D ? ? ? ? ?
        constexpr REL::Offset Handle_Add_Rf(0x6A9860);
        // 40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 C7 85 ? ? ? ? ? ? ? ? 0F 29 B4 24 ? ? ? ?
        constexpr REL::Offset BSTaskPool_HandleTask(0x5EAE40 + 0x2164);
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

    namespace GlobalTime
    {
        constexpr std::array todo = {
            REL::Offset(0x8843C0 + 0xB70), // BookMenu::vf4
            REL::Offset(0x8ECDB0 + 0x1BE) // SleepWaitMenu::vf4
        };
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
        constexpr REL::Offset BSBatchRenderer_SetupAndDrawPass(0x14142F580);
        constexpr REL::Offset BSLightingShader_vtbl(0x14196FFA0);
        
        constexpr REL::Offset BSLightingShader_SetupGeometry_ParallaxTechniqueLoc(0x1418820 + 0xB5D);
    }
}
// Patches

// Enable Achievements With Mods
// 48 83 EC 28 C6 44 24 ? ?
constexpr REL::ID AchievementModsEnabledFunction_offset(13647);

// Form Caching
constexpr REL::ID LookupFormByID_offset(14461);
constexpr REL::ID GlobalFormTableLock_offset(514360);
constexpr REL::ID GlobalFormTable_offset(514351);

// E8 ? ? ? ? 90 83 05 ? ? ? ? ? ->
constexpr REL::ID UnkFormFunc1_offset(14471);
// E8 ? ? ? ? 84 C0 75 36 48 8B CD ->
constexpr REL::ID UnkFormFunc2_offset(14515);
// E8 ? ? ? ? 40 B5 01 49 8B CD ->
constexpr REL::ID UnkFormFunc3_offset(14514);
// E8 ? ? ? ? 48 8D 84 24 ? ? ? ? 48 89 44 24 ? 48 8D 44 24 ? 48 89 44 24 ? 4C 8D 4F 14 -> +0xC4 ->
constexpr REL::ID UnkFormFunc4_offset(14537);

// Regular Quicksaves
// QuickSaveLoadHandler::HandleEvent  = vtbl 5
constexpr REL::ID QuickSaveLoadHandler_HandleEvent_SaveType_offset(51402);  // F0000200
constexpr REL::ID QuickSaveLoadHandler_HandleEvent_LoadType_offset(51402);  // D0000400

// Scrolling Doesn't Switch POV
// FirstPersonState::PlayerInputHandler::sub_4
// 48 39 08 75 0B
constexpr REL::ID FirstPersonState_DontSwitchPOV_offset(49800);
// TPS
// 74 35 48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0
constexpr REL::ID ThirdPersonState_DontSwitchPOV_offset(49970);

// Tree LOD Reference Caching
// E8 ? ? ? ? EB 0F 48 8B 43 18 ->
constexpr REL::ID UpdateBlockVisibility_orig_offset(30839);
// E8 ? ? ? ? 66 89 47 04 ->
constexpr REL::ID Float2Half_offset(74491);

// Waterflow
// E8 ? ? ? ? 84 DB 74 24 -> +0x252
constexpr REL::ID GameLoop_Hook_offset(35565);
constexpr REL::ID UnkGameLoopDword_offset(523662);
// 5th function in ??_7BSWaterShader@@6B@ vtbl
// F3 0F 10 0D ? ? ? ? F3 0F 11 4C 82 ?
constexpr REL::ID WaterShader_ReadTimer_Hook_offset(100602);

// Fixes

// Calendar Skipping
// E8 ? ? ? ? F6 87 DC 0B 00 00 01
constexpr REL::ID Calendar_AdvanceTime_call_offset(35402);

// Warnings

// Dupe Addon Node index
constexpr REL::ID vtbl_BGSAddonNode_LoadForm_offset(233357);

// E8 ? ? ? ? E9 ? ? ? ? 4C 8D 9C 24 80 00 00 00
constexpr REL::ID Unk_DataReload_Func_offset(35593);

constexpr REL::ID Call1_Unk_DataReload_func_offset(35551);

// E8 ? ? ? ? 33 C9 E8 ? ? ? ? 84 C0
constexpr REL::ID Call2_Unk_DataReload_func_offset(35589);

// Refr Handle Limit
// LookupRefrPtrByHandle uses this
constexpr REL::ID g_RefrHandleArray_offset(514478);

// Experimental

// BB ? ? ? ? 4C 8B FA
constexpr REL::ID Win32FileType_CopyToBuffer_offset(101985);
// C6 83 ? ? ? ? ? BA ? ? ? ? ->
constexpr REL::ID Win32FileType_ctor_offset(101962);
// E8 ? ? ? ? 8B D0 4C 8B CB ->
constexpr REL::ID ScrapHeap_GetMaxSize_offset(35203);

// E8 ? ? ? ? 0F B6 D0 EB 02
constexpr REL::ID TESFile_IsMaster_offset(13913);

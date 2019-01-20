#pragma once
// 1_5_62

// Patches

// E8 ? ? ? ? 84 DB 74 24 -> +0x230
constexpr uintptr_t g_FrameTimer_SlowTime_offset = 0x02F92948;
constexpr uintptr_t g_FrameTimer_NoSlowTime_offset = 0x02F9294C;

// Disable Chargen Precache
// E8 ? ? ? ? E8 ? ? ? ? 48 8D 57 30  ->
constexpr uintptr_t ChargenCacheFunction_offset = 0x008B2DD0;
// 90 E8 ? ? ? ? 90 48 8B 15 ? ? ? ?  ->
constexpr uintptr_t ChargenCacheClearFunction_offset = 0x008B2F50;

// Enable Achievements With Mods
constexpr uintptr_t AchievementModsEnabledFunction_offset = 0x0016F4D0;

// Form Caching
constexpr uintptr_t LookupFormByID_offset = 0x00194420;
constexpr uintptr_t GlobalFormTableLock_offset = 0x01EEB150;
constexpr uintptr_t GlobalFormTable_offset = 0x01EEACB8;

// E8 ? ? ? ? 90 83 05 ? ? ? ? ? ->
constexpr uintptr_t UnkFormFunc1_offset = 0x001949E0;
// E8 ? ? ? ? 84 C0 75 36 48 8B CD ->
constexpr uintptr_t UnkFormFunc2_offset = 0x001960E0;
// E8 ? ? ? ? 40 B5 01 49 8B CD ->
constexpr uintptr_t UnkFormFunc3_offset = 0x00195E10;
// E8 ? ? ? ? 48 8D 84 24 ? ? ? ? 48 89 44 24 ? 48 8D 44 24 ? 48 89 44 24 ? 4C 8D 4F 14 -> +0xC4 ->
constexpr uintptr_t UnkFormFunc4_offset = 0x001969D0;

// Regular Quicksaves
// QuickSaveLoadHandler::HandleEvent  = vtbl 5
constexpr uintptr_t quicksaveloadhandler_handleevent_savetype = 0x008AAB98;
constexpr uintptr_t quicksaveloadhandler_handleevent_loadtype = 0x008AABCB;

// Save Added Sound Categories
constexpr uintptr_t vtbl_BGSSoundCategory_offset = 0x01591030;
constexpr uintptr_t vtbl_BGSSoundCategory_LoadForm_offset = 0x01591060;
constexpr uintptr_t vtbl_BSISoundCategory_SetVolume_offset = 0x01591260;
constexpr uintptr_t vtbl_INIPrefSettingCollection_SaveFromMenu_offset = 0x0154FB28;

// Scrolling Doesn't Switch POV
// FirstPersonState::PlayerInputHandler::sub_4
constexpr uintptr_t FirstPersonState_DontSwitchPOV_offset = 0x00848013;
// TPS
constexpr uintptr_t ThirdPersonState_DontSwitchPOV_offset = 0x0084FFB8;

// Sleep Wait Time
// "getSliderValue" comiss
constexpr uintptr_t SleepWaitTime_Compare_offset = 0x008BDC7E;

// Tree LOD Reference Caching
// E8 ? ? ? ? EB 0F 48 8B 43 18 ->
constexpr uintptr_t UpdateBlockVisibility_orig_offset = 0x004A8280;
// E8 ? ? ? ? 66 89 47 04 ->
constexpr uintptr_t Float2Half_offset = 0x00D42430;


// Waterflow 
// E8 ? ? ? ? 84 DB 74 24 -> +0x252
constexpr uintptr_t GameLoop_Hook_offset = 0x005B3432;
constexpr uintptr_t UnkGameLoopDword_offset = 0x02F92950;
// 5th function in ??_7BSWaterShader@@6B@ vtbl
// F3 0F 10 0D ? ? ? ? F3 0F 11 4C 82 ?
constexpr uintptr_t WaterShader_ReadTimer_Hook_offset = 0x0130DC49;


// Fixes

// Double Perk Apply
// 48 85 D2 74 7C 48 89 5C 24 ? 
constexpr uintptr_t unknown_add_func = 0x005C6990;
// E8 ? ? ? ? B2 01 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? -> +0xA
constexpr uintptr_t handle_add_rf = 0x00682210;
// 44 0F B6 47 ? 48 8B D3 E8 ? ? ? ? 
constexpr uintptr_t switch_function_movzx = 0x005C8D1E;
// 41 0F B6 F9 48 8B C2 
constexpr uintptr_t unknown_add_function_movzx = 0x005C69AA;
// +0x2C
constexpr uintptr_t unknown_add_function_movzx2 = 0x005C69D6;
// E8 ? ? ? ? 48 8D 35 ? ? ? ? 48 89 5C 24 ? -> +0x12 -> +0x1B
constexpr uintptr_t next_formid_get_hook = 0x006821DB;
// called just after switch function movzx above, +0x1B
constexpr uintptr_t do_handle_hook = 0x003388AB;
// 74 1B 0F B6 42 08 
constexpr uintptr_t do_add_hook = 0x00338C51;

// Memory Access Errors
constexpr uintptr_t BSLightingShaderMaterialSnow_vtbl_offset = 0x0187A1C0;
constexpr uintptr_t BSLightingShader_SetupMaterial_Snow_Hook_offset = 0x013098D0;
constexpr uintptr_t BSLightingShader_SetupMaterial_Snow_Exit_offset = 0x013099A6;

constexpr uintptr_t vtbl_BGSShaderParticleGeometryData_LoadForm_offset = 0x0157A000; // vtbl[6]

// 48 8B C4 48 89 50 10 55  53 56 57 41 54 41 55 41  56 41 57 48 8D A8 38 FB
constexpr uintptr_t BadUseFuncBase_offset = 0x0133C590;

// MO5S Typo
constexpr uintptr_t MO5STypo_offset = 0x0019B503;

// PerkFragmentIsRunning
// 48 89 5C 24 08 57 48 83  EC 20 33 FF 49 8B D9 49  89 39 48 85 C9 ?? ?? 80  79 1A 3E 48 0F 44 F9 48  8B CF ?? ?? ?? ?? ?? 84
constexpr uintptr_t GameFunc_Native_IsRunning_offset = 0x002DB800;

// RemovedSpellBook
constexpr uintptr_t TESObjectBook_vtbl_offset = 0x01573318;

// Save Screenshots
// 84 C0 75 26 E8 ? ? ? ?  + 0x9
constexpr uintptr_t bgssaveloadmanager_processevents_requestscreenshot_hook = 0x00589DB3;
constexpr uintptr_t menusave_requestscreenshot_hook = 0x005B177A;
// 41 89 5d 00 40 84 ff 0f 85 + 0x8
constexpr uintptr_t screenshot_jnz_hook = 0x012AEA1A;
constexpr uintptr_t screenshot_render_orig_jnz = 0x012AECB5;
// + 0x128 from screenshot_jnz
constexpr uintptr_t render_target_hook_1 = 0x012AEB45;
// + 0x85
constexpr uintptr_t render_target_hook_2 = 0x012AEBCA;
constexpr uintptr_t g_requestSaveScreenshot = 0x02F5F968;

// Slow Time Camera Movement
// 40 53 48 83 EC 50 F3 0F 10 51 ? 48 8B D9 +0x2B, +0x92, +0x1F9
constexpr uintptr_t CameraMove_Timer1_offset = 0x008507BF; // +0x4
constexpr uintptr_t CameraMove_Timer2_offset = 0x00850826; // +0x4
constexpr uintptr_t CameraMove_Timer3_offset = 0x0085098D; // +0x4
// F3 0F 59 1D ? ? ? ? F3 0F 10 05 ? ? ? ?
constexpr uintptr_t CameraMove_Timer4_offset = 0x00850D2A; // +0x4
// E8 ? ? ? ? 48 8D 4B 4C -> +0x13
constexpr uintptr_t CameraMove_Timer5_offset = 0x00850DF7; // +0x4

// Tree Reflections
// ??_7BSDistantTreeShader@@6B@
// vfunc 3 -> +0x37
constexpr uintptr_t bsdistanttreeshader_hook = 0x013188C7;

// Warnings

// Dupe Addon Node index
constexpr uintptr_t vtbl_BGSAddonNode_LoadForm_offset = 0x015671B0;

// Refr Handle Limit
constexpr uintptr_t g_RefrHandleArray_offset = 0x01EEB7C0;

// Experimental

/// E8 ? ? ? ? 89 38  ->
constexpr uintptr_t MemoryManagerAlloc_offset = 0x00C02450;
// E8 ? ? ? ? 89 77 0C ->
constexpr uintptr_t MemoryManagerFree_offset = 0x00C02750;
// relative offsets to these are unlikely to change
constexpr uintptr_t ScrapHeapInit_offset = 0x00C03510;
constexpr uintptr_t ScrapHeapAlloc_offset = 0x00C03690;
constexpr uintptr_t ScrapHeapFree_offset = 0x00C03CB0;
constexpr uintptr_t ScrapHeapDeInit_offset = 0x00C03E70;
// E8 ? ? ? ? 90 0F AE F0 +0x102
constexpr uintptr_t InitMemoryManager_offset = 0x0059B7E0;
constexpr uintptr_t InitBSSmallBlockAllocator_offset = 0x0059B410;

//
constexpr uintptr_t TESFile_IsMaster_offset = 0x0017E320;
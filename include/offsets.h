#pragma once

#include "REL/Relocation.h"

// Patches

// E8 ? ? ? ? 84 DB 74 24 -> +0x230
constexpr REL::ID g_FrameTimer_SlowTime_offset(523660);
constexpr REL::ID g_FrameTimer_NoSlowTime_offset(523661);

// BSLightingAmbientSpecular
// 41 F7 85 ? ? ? ? ? ? ? ? 74 0A 
constexpr REL::ID AddAmbientSpecularToSetupGeometry_offset(100565);
// 49 8B 47 28 0F 10 05 ? ? ? ? 
constexpr REL::ID g_AmbientSpecularAndFresnel_offset(513256);
constexpr REL::ID DisableSetupMaterialAmbientSpecular_offset(100563);

// BSLightingShader Alpha
// E8 ? ? ? ? 49 8B 96 ? ? ? ? 40 B6 01 
constexpr REL::ID BSBatchRenderer_SetupAndDrawPass_offset(100854);
constexpr REL::ID BSLightingShader_vtbl_offset(305261);

// Disable Chargen Precache
// E8 ? ? ? ? E8 ? ? ? ? 48 8D 57 30  ->
constexpr REL::ID ChargenCacheFunction_offset(51507);
// 90 E8 ? ? ? ? 90 48 8B 15 ? ? ? ?  ->
constexpr REL::ID ChargenCacheClearFunction_offset(51509);

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
constexpr REL::ID QuickSaveLoadHandler_HandleEvent_SaveType_offset(51402); // F0000200
constexpr REL::ID QuickSaveLoadHandler_HandleEvent_LoadType_offset(51402); // D0000400

// Save Added Sound Categories
constexpr REL::ID vtbl_BGSSoundCategory_offset(236600);
constexpr REL::ID vtbl_BGSSoundCategory_BSISoundCategory_SetVolume_offset(236602); // vtbl 3
constexpr REL::ID vtbl_INIPrefSettingCollection_Unlock_offset(230546); // vtbl 6

// Scrolling Doesn't Switch POV
// FirstPersonState::PlayerInputHandler::sub_4
// 48 39 08 75 0B 
constexpr REL::ID FirstPersonState_DontSwitchPOV_offset(49800);
// TPS
// 74 35 48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 
constexpr REL::ID ThirdPersonState_DontSwitchPOV_offset(49970);

// Sleep Wait Time
// "getSliderValue" comiss
constexpr REL::ID SleepWaitTime_Compare_offset(51614);

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

// Archery Downward Aiming
			// E8 ? ? ? ? 8B 83 CC 01 00 00 C1 E8 12
constexpr REL::ID CalculateCollisionCall_offset(42852);

// Double Perk Apply
// 48 85 D2 74 7C 48 89 5C 24 ? 
constexpr REL::ID QueueApplyPerk_offset(36007);
// E8 ? ? ? ? B2 01 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? -> +0xA
constexpr REL::ID Handle_Add_Rf_offset(38967);
// 44 0F B6 47 ? 48 8B D3 E8 ? ? ? ? 
constexpr REL::ID Switch_Function_movzx_offset(36016);
// 41 0F B6 F9 48 8B C2 
constexpr REL::ID Unknown_Add_Function_movzx_offset(36007);
// +0x2C
constexpr REL::ID Unknown_Add_Function_movzx2_offset(36007);
// E8 ? ? ? ? 48 8D 35 ? ? ? ? 48 89 5C 24 ? -> +0x12 -> +0x1B
constexpr REL::ID Next_Formid_Get_Hook_offset(38966);
// called just after switch function movzx above, +0x1B
constexpr REL::ID Do_Handle_Hook_offset(23353);
// 74 1B 0F B6 42 08 
constexpr REL::ID Do_Add_Hook_offset(23359);

// Equip Shout Event Spam
// E8 ? ? ? ? F6 86 ? ? ? ? ? 74 0F 
constexpr REL::ID Equip_Shout_Procedure_Function_offset(37821);

// GHeap Leak Detection Crash
// E8 ? ? ? ? 48 8B 07 33 D2 48 8B CF FF 10
constexpr REL::ID GHeap_Leak_Detection_Crash_offset(85757);

// Lip Sync Fix
// E8 ? ? ? ? 48 8D 8F 20 01 00 00 0F 28 CE
constexpr REL::ID LipSync_FUNC_ADDR(16023);

// Memory Access Errors
constexpr REL::ID BSLightingShaderMaterialSnow_vtbl_offset(304565);
// 49 8B 4F 28 F3 0F 10 93 ? ? ? ? F3 0F 10 8B ? ? ? ? F3 0F 10 83 ? ? ? ? 
constexpr REL::ID BSLightingShader_SetupMaterial_Snow_Hook_offset(100563);
constexpr REL::ID BSLightingShader_SetupMaterial_Snow_Exit_offset(100563);

constexpr REL::ID vtbl_BGSShaderParticleGeometryData_LoadForm_offset(234671); // vtbl[6]

// 48 8B C4 48 89 50 10 55  53 56 57 41 54 41 55 41  56 41 57 48 8D A8 38 FB
constexpr REL::ID BadUseFuncBase_offset(101499);

// MO5S Typo
// 3D ? ? ? ? 74 18 8B C8 
constexpr REL::ID MO5STypo_offset(14653);

// PerkFragmentIsRunning
// 48 89 5C 24 08 57 48 83  EC 20 33 FF 49 8B D9 49  89 39 48 85 C9 ?? ?? 80  79 1A 3E 48 0F 44 F9 48  8B CF ?? ?? ?? ?? ?? 84
constexpr REL::ID GameFunc_Native_IsRunning_offset(21119);

// RemovedSpellBook
constexpr REL::ID TESObjectBook_vtbl_offset(234122);

// Save Screenshots
// 84 C0 75 26 E8 ? ? ? ?  + 0x9
constexpr REL::ID BGSSaveLoadManager_ProcessEvents_RequestScreenshot_hook_offset(34862);
constexpr REL::ID MenuSave_RequestScreenshot_hook_offset(35556);
// 41 89 5d 00 40 84 ff 0f 85 + 0x8
constexpr REL::ID Screenshot_Jnz_hook_offset(99023);
constexpr REL::ID Screenshot_Render_Orig_jnz_offset(99023);
// + 0x128 from screenshot_jnz
constexpr REL::ID Render_Target_Hook_1_offset(99023);
// + 0x85
constexpr REL::ID Render_Target_Hook_2_offset(99023);
constexpr REL::ID g_RequestSaveScreenshot_offset(517224);

// Slow Time Camera Movement
// 40 53 48 83 EC 50 F3 0F 10 51 ? 48 8B D9 +0x2B, +0x92, +0x1F9
constexpr REL::ID CameraMove_Timer1_offset(49977); // +0x4
constexpr REL::ID CameraMove_Timer2_offset(49977); // +0x4
constexpr REL::ID CameraMove_Timer3_offset(49977); // +0x4
// F3 0F 59 1D ? ? ? ? F3 0F 10 05 ? ? ? ?
constexpr REL::ID CameraMove_Timer4_offset(49980); // +0x4
// E8 ? ? ? ? 48 8D 4B 4C -> +0x13
constexpr REL::ID CameraMove_Timer5_offset(49981); // +0x4

// Calendar Skipping
// E8 ? ? ? ? F6 87 DC 0B 00 00 01
constexpr REL::ID Calendar_AdvanceTime_call_offset(35402);

// Tree Reflections
// ??_7BSDistantTreeShader@@6B@
// vfunc 3 -> +0x37
constexpr REL::ID BSDistantTreeShader_hook_offset(100771);

// Vertical Look Sensitivity
// ??_7ThirdPersonState@@6B@ vtbl last function + 0x71
constexpr REL::ID ThirdPersonState_Vfunc_Hook_offset(49978);
// ??_7DragonCameraState@@6B@ vtbl last function + 0x5F
constexpr REL::ID DragonCameraState_Vfunc_Hook_offset(32370);
// ??_7HorseCameraState@@6B@ vtbl last function + 0x5F
constexpr REL::ID HorseCameraState_Vfunc_Hook_offset(49839);

// ??_7EnchantmentItem@@6B@
constexpr REL::ID offset_vtbl_EnchantmentItem(228570);

// Animation Load Sign
// SkyrimSE.exe+0x00B669C0    | 0FBF83 28010000          | movsx   eax, word ptr ds:[rbx + 0x128]            |
//-> SkyrimSE.exe+0x00B669C0  | 0FB783 28010000          | movzx   eax, word ptr ds:[rbx + 0x128]            |
constexpr REL::ID offset_AnimationLoadSigned(64198);

// BSLightingShader::SetupGeometry Parallax Technique fix
// 8B C1 25 ? ? ? ? 41 0F 45 D0 
constexpr REL::ID offset_BSLightingShader_SetupGeometry_ParallaxTechniqueFix(100565);

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

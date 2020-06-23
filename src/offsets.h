#pragma once

// Patches

// E8 ? ? ? ? 84 DB 74 24 -> +0x230
constexpr REL::ID g_FrameTimer_SlowTime_offset(523660);
constexpr REL::ID g_FrameTimer_NoSlowTime_offset(523661);

// BSLightingShader Alpha
// E8 ? ? ? ? 49 8B 96 ? ? ? ? 40 B6 01
constexpr REL::ID BSBatchRenderer_SetupAndDrawPass_offset(100854);
constexpr REL::ID BSLightingShader_vtbl_offset(305261);

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

// Save Added Sound Categories
constexpr REL::ID vtbl_BGSSoundCategory_offset(236600);
constexpr REL::ID vtbl_BGSSoundCategory_BSISoundCategory_SetVolume_offset(236602);  // vtbl 3
constexpr REL::ID vtbl_INIPrefSettingCollection_Unlock_offset(230546);              // vtbl 6

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

// Calendar Skipping
// E8 ? ? ? ? F6 87 DC 0B 00 00 01
constexpr REL::ID Calendar_AdvanceTime_call_offset(35402);

// BSLightingShader::SetupGeometry Parallax Technique fix
// 8B C1 25 ? ? ? ? 41 0F 45 D0
constexpr REL::ID offset_BSLightingShader_SetupGeometry_ParallaxTechniqueFix(100565);

// Fix for crash somewhere called from CreateArmorNode
constexpr REL::ID offset_CreateArmorNode_subfunc(15535);

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

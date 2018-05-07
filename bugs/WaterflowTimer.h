#pragma once
#include "../skse64_common/Relocation.h"

bool PatchWaterflowTimer();

// offsets
// sse 1.5.39

#define off_GAMELOOP_AFTER_UPDATEGAMEHOUR		0x005B36F2
#define off_UNKNOWN_GAMELOOP_DWORD				0x02F92950

#define off_TIMER_STEP_FLOAT					0x02F92948

#define off_WATERSHADER_READTIMER				0x0130DFD9

// patch addresses

extern RelocAddr<uintptr_t> GameLoop_Hook;
extern RelocPtr<uint32_t> UnkGameLoopDword;

extern RelocPtr<float> TimerStep;

extern RelocAddr<uintptr_t> WaterShader_ReadTimer_Hook;
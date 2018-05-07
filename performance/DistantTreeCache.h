#pragma once

#include "../skse64_common/Relocation.h"
#include "../TES/BSTArray.h"

// https://github.com/Nukem9/SykrimSETest/blob/master/skyrim64_test/src/patches/TES/BGSDistantTreeBlock.h

struct LODGroupInstance
{
	uint32_t FormId;	// Only the lower 24 bits used
	char _pad[0xA];
	uint16_t Alpha;		// This is Float2Word(fAlpha)
	bool Hidden;		// Alpha <= 0.0f or set by object flags
};

struct LODGroup
{
	char _pad[8];
	BSTArray<LODGroupInstance> m_LODInstances;
	char _pad2[4];
	bool m_UnkByte24;
};

struct ResourceData
{
	BSTArray<LODGroup *> m_LODGroups;
	char _pad[106];
	bool m_UnkByte82;
};

bool PatchDistantTreeCache();
void InvalidateCachedForm(uint32_t formId);

// offsets
// sse 1.5.39

#define off_BGSDISTANTTREEFADE_UPDATELODALPHAFADE	0x004A8440
#define off_FLOAT2HALF								0x00D42750 

// hooks

typedef void(*UpdateLODAlphaFade_)(ResourceData * data);
void UpdateLODAlphaFade_Hook(ResourceData * data);
extern RelocAddr<UpdateLODAlphaFade_> UpdateLODAlphaFade_orig;

typedef uint16_t(*Float2Half_)(float f);
extern RelocAddr<Float2Half_> Float2Half;





#pragma once
#include "../tes/BSTArray.h"

namespace FormCaching
{
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

	#define TES_FORM_MASTER_COUNT	256			// Maximum master file index + 1 (2^8, 8 bits)
	#define TES_FORM_INDEX_COUNT	16777216	// Maximum index + 1 (2^24, 24 bits)

	bool Patch();
}

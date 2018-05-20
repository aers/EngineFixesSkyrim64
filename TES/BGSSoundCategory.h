#pragma once

#include "../../skse64/GameForms.h"

namespace TES
{
	// 58 
	class BGSSoundCategory : public TESForm
	{
	public:
		enum { kTypeID = kFormType_SoundCategory };

		// parents
		TESFullName			fullName;		// 20
		BSISoundCategory	soundCategory;	// 30

										// members
		UInt32	flags;					// 38 - init'd to 0			flags (FNAM)
		UInt32  unk3C;					// 3C - unk					???
		BGSSoundCategory * parent;		// 40 - init'd to 0			pointer to parent sound category (SNCT)
		UInt16	unk48;					// 48 - init'd to 0			??? maybe some sort of flag its set to 02 on a few
		UInt16	unk4A;					// 4A - init'd to 0			???
		UInt16	staticVolumeMultiplier;	// 4C - init'd to FFFF		static volume multiplier (VNAM) 
		UInt16	defaultMenuValue;	;	// 4E - init'd to FFFF		default menu value (UNAM)
		float	ingameVolume;			// 50 - init'd to 1			ingame volume slider
		float	unk54;					// 54 - init'd to 1			???
	};
}
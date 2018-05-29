#include "../../skse64_common/Relocation.h"
#include "../../skse64_common/SafeWrite.h"

namespace PrecacheKiller
{
	// E8 ? ? ? ? E8 ? ? ? ? 48 8D 57 30  ->
	RelocAddr<uintptr_t> PrecacheChargen(0x008B3090);
	// 90 E8 ? ? ? ? 90 48 8B 15 ? ? ? ?  ->
	RelocAddr<uintptr_t> PrecacheChargenClear(0x008B3210);

	bool Patch()
	{
		_MESSAGE("-- precache killer --");

		_MESSAGE("patching precache chargen");
		SafeWrite8(PrecacheChargen.GetUIntPtr(), 0xC3);
		_MESSAGE("patching precache chargen clear");
		SafeWrite8(PrecacheChargenClear.GetUIntPtr(), 0xC3);

		_MESSAGE("success");

		return true;
	}
}
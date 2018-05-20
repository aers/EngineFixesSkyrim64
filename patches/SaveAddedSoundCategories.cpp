#include "../../skse64_common/SafeWrite.h"
#include "../../skse64/GameForms.h"

#include "../TES/BGSSoundCategory.h"
#include <cinttypes>

namespace SaveAddedSoundCategories
{
	typedef bool(*_BGSSoundCategory_LoadForm)(TES::BGSSoundCategory * soundCategory, int64_t unk1);
	RelocAddr<_BGSSoundCategory_LoadForm> BGSSoundCategory_LoadForm(0x002CDB60);
	RelocAddr<uintptr_t> vtbl_BGSSoundCategory_LoadForm(0x01591050); // ::LoadForm = vtable[6] in TESForm derived classes

	// unk1 here is probably a pointer to the file object its loading from, but that's not super important
	bool hk_BGSSoundCategory_LoadForm(TES::BGSSoundCategory * soundCategory, int64_t unk1)
	{
		bool result = BGSSoundCategory_LoadForm(soundCategory, unk1);

		if (result)
		{
			_MESSAGE("BGSSoundCategory_LoadForm(0x%016" PRIXPTR ", 0x%016" PRIXPTR ") - loaded sound category for formid %08X and name %s", soundCategory, unk1, soundCategory->formID, soundCategory->fullName.GetName());
		}
		else
		{
			_MESSAGE("sound category load error????");
		}

		return result;
	}

	bool Patch()
	{
		SafeWrite64(vtbl_BGSSoundCategory_LoadForm.GetUIntPtr(), GetFnAddr(hk_BGSSoundCategory_LoadForm));
		return true;
	}
}

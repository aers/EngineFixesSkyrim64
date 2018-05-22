#include "../../skse64_common/SafeWrite.h"
#include "../../skse64/GameForms.h"
#include "../../skse64/GameData.h"

#include "../TES/BGSSoundCategory.h"
#include <cinttypes>

namespace SaveAddedSoundCategories
{
	typedef bool(*_BGSSoundCategory_LoadForm)(TES::BGSSoundCategory * soundCategory, ModInfo * modInfo);
	RelocAddr<_BGSSoundCategory_LoadForm> BGSSoundCategory_LoadForm(0x002CDB60);
	RelocAddr<uintptr_t> vtbl_BGSSoundCategory_LoadForm(0x01591050); // ::LoadForm = vtable[6] in TESForm derived classes

	typedef bool(*_INIPrefSettingCollection_SaveFromMenu)(__int64 thisPtr, __int64 unk1, char * fileName, __int64 unk2);
	RelocAddr<_INIPrefSettingCollection_SaveFromMenu> INIPrefSettingCollection_SaveFromMenu(0x00C10880);
	RelocAddr<uintptr_t> vtbl_INIPrefSettingCollection_SaveFromMenu(0x0154FB18); // ::SaveFromMenu??? = vtable[8]

	bool hk_INIPrefSettingCollection_SaveFromMenu(__int64 thisPtr, __int64 unk1, char * fileName, __int64 unk2)
	{
		const bool retVal = INIPrefSettingCollection_SaveFromMenu(thisPtr, unk1, fileName, unk2);

		_MESSAGE("SaveFromMenu called filename %s", fileName);

		return retVal;
	}

	bool hk_BGSSoundCategory_LoadForm(TES::BGSSoundCategory * soundCategory, ModInfo * modInfo)
	{
		const bool result = BGSSoundCategory_LoadForm(soundCategory, modInfo);

		if (result)
		{
			_MESSAGE("BGSSoundCategory_LoadForm(0x%016" PRIXPTR ", 0x%016" PRIXPTR ") - loaded sound category for formid %08X and name %s from plugin filename %s", soundCategory, modInfo, soundCategory->formID, soundCategory->fullName.GetName(), modInfo->name);
		}
		else
		{
			_MESSAGE("sound category load error????");
		}

		return result;
	}

	bool Patch()
	{
		SafeWrite64(vtbl_INIPrefSettingCollection_SaveFromMenu.GetUIntPtr(), GetFnAddr(hk_INIPrefSettingCollection_SaveFromMenu));
		SafeWrite64(vtbl_BGSSoundCategory_LoadForm.GetUIntPtr(), GetFnAddr(hk_BGSSoundCategory_LoadForm));
		return true;
	}
}

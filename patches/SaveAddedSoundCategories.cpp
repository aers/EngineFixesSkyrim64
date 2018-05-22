#include "../../skse64_common/Utilities.h"
#include "../../skse64_common/SafeWrite.h"
#include "../../skse64/GameForms.h"
#include "../../skse64/GameData.h"

#include "../TES/BGSSoundCategory.h"
#include "SaveAddedSoundCategories.h"
#include "../lib/Simpleini.h"

#include <map>
#include <cinttypes>


namespace SaveAddedSoundCategories
{
	CSimpleIniA snctIni;
	std::map<uint32_t, SoundCategoryInfo> soundCategories;

	typedef bool(*_BGSSoundCategory_LoadForm)(TES::BGSSoundCategory * soundCategory, ModInfo * modInfo);
	RelocAddr<_BGSSoundCategory_LoadForm> BGSSoundCategory_LoadForm(0x002CDB60);
	RelocAddr<uintptr_t> vtbl_BGSSoundCategory_LoadForm(0x01591050); // ::LoadForm = vtable[6] in TESForm derived classes

	typedef bool(*_BSISoundCategory_SetVolume)(BSISoundCategory * thisPtr, float volume);
	RelocAddr<_BSISoundCategory_SetVolume> BSISoundCategory_SetVolume(0x002CE090); // ::SetVolume = vtable[3] in ??_7BGSSoundCategory@@6B@_1 (BSISoundCategory)

	typedef bool(*_INIPrefSettingCollection_SaveFromMenu)(__int64 thisPtr, __int64 unk1, char * fileName, __int64 unk2);
	RelocAddr<_INIPrefSettingCollection_SaveFromMenu> INIPrefSettingCollection_SaveFromMenu(0x00C10880);
	RelocAddr<uintptr_t> vtbl_INIPrefSettingCollection_SaveFromMenu(0x0154FB18); // ::SaveFromMenu??? = vtable[8]

	bool hk_INIPrefSettingCollection_SaveFromMenu(__int64 thisPtr, __int64 unk1, char * fileName, __int64 unk2)
	{
		const bool retVal = INIPrefSettingCollection_SaveFromMenu(thisPtr, unk1, fileName, unk2);

		//_MESSAGE("SaveFromMenu called filename %s", fileName);

		for (auto& soundCategory : soundCategories)
		{
			char localFormIdHex[9];
			sprintf_s(localFormIdHex, sizeof(localFormIdHex), "%08X", soundCategory.second.LocalFormId);
			snctIni.SetDoubleValue(soundCategory.second.PluginName.c_str(), localFormIdHex, static_cast<double>(soundCategory.second.Category->ingameVolume),
			                       soundCategory.second.Category->GetName(), true);
		}

		const std::string& runtimePath = GetRuntimeDirectory();

		const SI_Error saveRes = snctIni.SaveFile((runtimePath + R"(Data\SKSE\plugins\EngineFixes64_SNCT.ini)").c_str());

		if (saveRes < 0)
		{
			_MESSAGE("warning: unable to save snct ini");
		}

		return retVal;
	}

	bool hk_BGSSoundCategory_LoadForm(TES::BGSSoundCategory * soundCategory, ModInfo * modInfo)
	{
		const bool result = BGSSoundCategory_LoadForm(soundCategory, modInfo);

		if (result)
		{
			//_MESSAGE("[%s] BGSSoundCategory_LoadForm(0x%016" PRIXPTR ", 0x%016" PRIXPTR ") - loaded sound category for formid %08X and name %s", modInfo->name, soundCategory, modInfo, soundCategory->formID, soundCategory->fullName.GetName());
			if (soundCategory->flags & 0x2)
			{
				//_MESSAGE("menu flag set, flagging for save");
				uint32_t localFormId = soundCategory->formID & 0x00FFFFFF;
				// esl
				if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
				{
					localFormId = localFormId & 0x00000FFF;
				}
				SoundCategoryInfo snct(modInfo->name, localFormId, soundCategory);
				soundCategories.insert(std::make_pair(soundCategory->formID, snct));
			}
			else
			{
				//_MESSAGE("menu flag not set, unflagging for save if form was flagged for save in prior plugin");
				soundCategories.erase(soundCategory->formID);
			}
		}
		else
		{
			_MESSAGE("sound category load error????");
		}

		return result;
	}

	void LoadVolumes()
	{
		//_MESSAGE("game has loaded, setting volumes");
		for (auto& soundCategory : soundCategories)
		{
			char localFormIdHex[9];
			sprintf_s(localFormIdHex, sizeof(localFormIdHex), "%08X", soundCategory.second.LocalFormId);
			const auto vol = snctIni.GetDoubleValue(soundCategory.second.PluginName.c_str(), localFormIdHex, -1.0);

			if (vol != -1.0)
			{
				//_MESSAGE("setting volume for formid %08X", soundCategory.second.Category->formID);
				BSISoundCategory * soundCatInterface = &soundCategory.second.Category->soundCategory;

				BSISoundCategory_SetVolume(soundCatInterface, static_cast<float>(vol));
			}
		}
	}

	bool Patch()
	{
		_MESSAGE("- save added sound categories -");

		const std::string& runtimePath = GetRuntimeDirectory();

		const SI_Error loadRes = snctIni.LoadFile((runtimePath + R"(Data\SKSE\plugins\EngineFixes64_SNCT.ini)").c_str());

		if (loadRes < 0)
		{
			_MESSAGE("unable to load SNCT ini, disabling patch");
			return false;
		}

		_MESSAGE("hooking vtbls");
		SafeWrite64(vtbl_INIPrefSettingCollection_SaveFromMenu.GetUIntPtr(), GetFnAddr(hk_INIPrefSettingCollection_SaveFromMenu));
		SafeWrite64(vtbl_BGSSoundCategory_LoadForm.GetUIntPtr(), GetFnAddr(hk_BGSSoundCategory_LoadForm));
		_MESSAGE("success");
		return true;
	}
}

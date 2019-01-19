#include "RE/BGSSoundCategory.h"
#include "RE/TESFile.h"

#include "patches.h"
#include "Simpleini.h"


namespace patches
{
    struct SoundCategoryInfo
    {
        std::string PluginName;
        uint32_t LocalFormId;
        RE::BGSSoundCategory * Category;

        SoundCategoryInfo(std::string name, uint32_t formid, RE::BGSSoundCategory * cat) : PluginName(name), LocalFormId(formid), Category(cat)
        {
        }
    };

    CSimpleIniA snctIni;
    std::map<uint32_t, SoundCategoryInfo> soundCategories;

    RelocPtr<uintptr_t> vtbl_BGSSoundCategory(vtbl_BGSSoundCategory_offset);

    typedef bool(*_BGSSoundCategory_LoadForm)(RE::BGSSoundCategory * soundCategory, RE::TESFile * modInfo);
    RelocPtr<_BGSSoundCategory_LoadForm> vtbl_BGSSoundCategory_LoadForm(vtbl_BGSSoundCategory_LoadForm_offset); // ::LoadForm = vtable[6] in TESForm derived classes
    _BGSSoundCategory_LoadForm orig_BGSSoundCategory_LoadForm;

    typedef bool(*_BSISoundCategory_SetVolume)(BSISoundCategory * thisPtr, float volume);
    RelocPtr<_BSISoundCategory_SetVolume> vtbl_BSISoundCategory_SetVolume(vtbl_BSISoundCategory_SetVolume_offset); // ::SetVolume = vtable[3] in ??_7BGSSoundCategory@@6B@_1 (BSISoundCategory)

    typedef bool(*_INIPrefSettingCollection_SaveFromMenu)(__int64 thisPtr, __int64 unk1, char * fileName, __int64 unk2);
    RelocPtr<_INIPrefSettingCollection_SaveFromMenu> vtbl_INIPrefSettingCollection_SaveFromMenu(vtbl_INIPrefSettingCollection_SaveFromMenu_offset); // ::SaveFromMenu??? = vtable[8]
    _INIPrefSettingCollection_SaveFromMenu orig_INIPrefSettingCollection_SaveFromMenu;

    bool hk_INIPrefSettingCollection_SaveFromMenu(__int64 thisPtr, __int64 unk1, char * fileName, __int64 unk2)
    {
        const bool retVal = orig_INIPrefSettingCollection_SaveFromMenu(thisPtr, unk1, fileName, unk2);

        // _MESSAGE("SaveFromMenu called filename %s", fileName);

        for (auto& soundCategory : soundCategories)
        {
            char localFormIdHex[9];
            sprintf_s(localFormIdHex, sizeof(localFormIdHex), "%08X", soundCategory.second.LocalFormId);

            if (*(uintptr_t *)soundCategory.second.Category != vtbl_BGSSoundCategory.GetUIntPtr())
            {
                // game's probably shutting down here
                _VMESSAGE("SNCT save: skipping save due to game closing");
                return retVal;
            }

            auto name = dynamic_cast<TESFullName*>(soundCategory.second.Category)->GetName();
            snctIni.SetDoubleValue(soundCategory.second.PluginName.c_str(), localFormIdHex, static_cast<double>(soundCategory.second.Category->ingameVolume));
        }

        const std::string& runtimePath = GetRuntimeDirectory();

        const SI_Error saveRes = snctIni.SaveFile((runtimePath + R"(Data\SKSE\plugins\EngineFixes_SNCT.ini)").c_str());

        if (saveRes < 0)
        {
            _VMESSAGE("warning: unable to save snct ini");
        }

        _VMESSAGE("SNCT save: saved sound categories");

        return retVal;
    }

    bool hk_BGSSoundCategory_LoadForm(RE::BGSSoundCategory * soundCategory, RE::TESFile * modInfo)
    {
        const bool result = orig_BGSSoundCategory_LoadForm(soundCategory, modInfo);

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
            _VMESSAGE("sound category load error????");
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
                BSISoundCategory * soundCatInterface = dynamic_cast<BSISoundCategory *>(soundCategory.second.Category);

                (*vtbl_BSISoundCategory_SetVolume)(soundCatInterface, static_cast<float>(vol));
            }
        }
    }

    bool PatchSaveAddedSoundCategories()
    {
        _VMESSAGE("- save added sound categories -");

        const std::string& runtimePath = GetRuntimeDirectory();

        const SI_Error loadRes = snctIni.LoadFile((runtimePath + R"(Data\SKSE\plugins\EngineFixes_SNCT.ini)").c_str());

        if (loadRes < 0)
        {
            _VMESSAGE("unable to load SNCT ini, disabling patch");
            return false;
        }

        _VMESSAGE("hooking vtbls");
        orig_INIPrefSettingCollection_SaveFromMenu = *vtbl_INIPrefSettingCollection_SaveFromMenu;
        SafeWrite64(vtbl_INIPrefSettingCollection_SaveFromMenu.GetUIntPtr(), GetFnAddr(hk_INIPrefSettingCollection_SaveFromMenu));
        orig_BGSSoundCategory_LoadForm = *vtbl_BGSSoundCategory_LoadForm;
        SafeWrite64(vtbl_BGSSoundCategory_LoadForm.GetUIntPtr(), GetFnAddr(hk_BGSSoundCategory_LoadForm));
        _VMESSAGE("success");
        return true;
    }
}

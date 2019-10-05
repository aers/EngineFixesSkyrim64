#include "RE/BGSSoundCategory.h"
#include "RE/TESFile.h"

#include "patches.h"
#include "Simpleini.h"
#include "RE/TESDataHandler.h"

#include "utils.h"


namespace patches
{
    CSimpleIniA snctIni;

    RelocPtr<uintptr_t> vtbl_BGSSoundCategory(vtbl_BGSSoundCategory_offset);

    typedef bool(*_BSISoundCategory_SetVolume)(RE::BSISoundCategory * thisPtr, float volume);
    RelocPtr<_BSISoundCategory_SetVolume> vtbl_BSISoundCategory_SetVolume(vtbl_BGSSoundCategory_BSISoundCategory_SetVolume_offset); // ::SetVolume = vtable[3] in ??_7BGSSoundCategory@@6B@_1 (BSISoundCategory)

    typedef bool(*_INIPrefSettingCollection_Unlock)(__int64 thisPtr);
    RelocPtr<_INIPrefSettingCollection_Unlock> vtbl_INIPrefSettingCollection_Unlock(vtbl_INIPrefSettingCollection_Unlock_offset);
    _INIPrefSettingCollection_Unlock orig_INIPrefSettingCollection_Unlock;

    bool hk_INIPrefSettingCollection_Unlock(__int64 thisPtr)
    {        
        const auto dataHandler = RE::TESDataHandler::GetSingleton();

        if (dataHandler)
        {
            for (auto& soundCategory : dataHandler->GetFormArray<RE::BGSSoundCategory>())
            {
                if ((soundCategory->flags & RE::BGSSoundCategory::Flag::kShouldAppearOnMenu) != RE::BGSSoundCategory::Flag::kNone)
                {
                    _VMESSAGE("processing %s", dynamic_cast<RE::TESFullName *>(soundCategory)->GetName());
                    _VMESSAGE("menu flag set, saving");
                    auto localFormId = soundCategory->formID & 0x00FFFFFF;
                    // esl
                    if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                    {
                        localFormId = localFormId & 0x00000FFF;
                    }
                    _VMESSAGE("plugin: %s form id: %08X", soundCategory->sourceFiles->files[0]->name, localFormId);

                    char localFormIdHex[9];
                    sprintf_s(localFormIdHex, sizeof(localFormIdHex), "%08X", localFormId);

                    snctIni.SetDoubleValue(soundCategory->sourceFiles->files[0]->name, localFormIdHex, static_cast<double>(soundCategory->ingameVolume));
                }
            }

            const std::string& runtimePath = GetRuntimeDirectory();

            const SI_Error saveRes = snctIni.SaveFile((runtimePath + R"(Data\SKSE\plugins\EngineFixes_SNCT.ini)").c_str());

            if (saveRes < 0)
            {
                _VMESSAGE("warning: unable to save snct ini");
            }

            _VMESSAGE("SNCT save: saved sound categories");
        }
        else
        {
            _VMESSAGE("SNCT save: data handler not ready, not saving sound categories for this call");
        }
        
        return orig_INIPrefSettingCollection_Unlock(thisPtr);
    }

    void LoadVolumes()
    {
        _VMESSAGE("game has loaded, setting volumes");

        const auto dataHandler = RE::TESDataHandler::GetSingleton();
        if (dataHandler)
        {
            for (auto& soundCategory : dataHandler->GetFormArray<RE::BGSSoundCategory>())
            {
                auto localFormId = soundCategory->formID & 0x00FFFFFF;
                // esl
                if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                {
                    localFormId = localFormId & 0x00000FFF;
                }
                char localFormIdHex[9];
                sprintf_s(localFormIdHex, sizeof(localFormIdHex), "%08X", localFormId);

                const auto vol = snctIni.GetDoubleValue(soundCategory->sourceFiles->files[0]->name, localFormIdHex, -1.0);

                if (vol != -1.0)
                {
                    _VMESSAGE("setting volume for formid %08X", soundCategory->formID);
                    const auto soundCatInterface = dynamic_cast<RE::BSISoundCategory *>(soundCategory);

                    (*vtbl_BSISoundCategory_SetVolume)(soundCatInterface, static_cast<float>(vol));
                }
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
        orig_INIPrefSettingCollection_Unlock = *vtbl_INIPrefSettingCollection_Unlock;
        SafeWrite64(vtbl_INIPrefSettingCollection_Unlock.GetUIntPtr(), GetFnAddr(hk_INIPrefSettingCollection_Unlock));
        _VMESSAGE("success");
        return true;
    }
}

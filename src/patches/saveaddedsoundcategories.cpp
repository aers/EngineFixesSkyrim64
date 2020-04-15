#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/SafeWrite.h"

#include "Simpleini.h"
#include "patches.h"

#include "utils.h"

namespace patches
{
    CSimpleIniA snctIni;

    REL::Offset<std::uintptr_t*> vtbl_BGSSoundCategory(vtbl_BGSSoundCategory_offset);

    typedef bool (*_BSISoundCategory_SetVolume)(RE::BSISoundCategory* thisPtr, float volume);
    REL::Offset<_BSISoundCategory_SetVolume*> vtbl_BSISoundCategory_SetVolume(vtbl_BGSSoundCategory_BSISoundCategory_SetVolume_offset, 0x8 * 0x3);  // ::SetVolume = vtable[3] in ??_7BGSSoundCategory@@6B@_1 (BSISoundCategory)

    typedef bool (*_INIPrefSettingCollection_Unlock)(__int64 thisPtr);
    REL::Offset<_INIPrefSettingCollection_Unlock*> vtbl_INIPrefSettingCollection_Unlock(vtbl_INIPrefSettingCollection_Unlock_offset, 0x8 * 0x6);
    _INIPrefSettingCollection_Unlock orig_INIPrefSettingCollection_Unlock;

    bool hk_INIPrefSettingCollection_Unlock(__int64 thisPtr)
    {
        const auto dataHandler = RE::TESDataHandler::GetSingleton();

        if (dataHandler)
        {
            for (auto& soundCategory : dataHandler->GetFormArray<RE::BGSSoundCategory>())
            {
                if (soundCategory->IsMenuCategory())
                {
                    auto fullName = soundCategory->GetFullName();
                    fullName = fullName ? fullName : "";
                    _VMESSAGE("processing %s", fullName);
                    _VMESSAGE("menu flag set, saving");
                    auto localFormId = soundCategory->formID & 0x00FFFFFF;
                    // esl
                    if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                    {
                        localFormId = localFormId & 0x00000FFF;
                    }
                    auto srcFile = soundCategory->GetDescriptionOwnerFile();
                    _VMESSAGE("plugin: %s form id: %08X", srcFile->fileName, localFormId);

                    char localFormIdHex[9];
                    sprintf_s(localFormIdHex, sizeof(localFormIdHex), "%08X", localFormId);

                    snctIni.SetDoubleValue(srcFile->fileName, localFormIdHex, static_cast<double>(soundCategory->volumeMult));
                }
            }

            const SI_Error saveRes = snctIni.SaveFile(R"(.\Data\SKSE\plugins\EngineFixes_SNCT.ini)");

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

                auto srcFile = soundCategory->GetDescriptionOwnerFile();
                const auto vol = snctIni.GetDoubleValue(srcFile->fileName, localFormIdHex, -1.0);

                if (vol != -1.0)
                {
                    _VMESSAGE("setting volume for formid %08X", soundCategory->formID);
                    const auto soundCatInterface = dynamic_cast<RE::BSISoundCategory*>(soundCategory);

                    (*vtbl_BSISoundCategory_SetVolume)(soundCatInterface, static_cast<float>(vol));
                }
            }
        }
    }

    bool PatchSaveAddedSoundCategories()
    {
        _VMESSAGE("- save added sound categories -");

        const SI_Error loadRes = snctIni.LoadFile(R"(.\Data\SKSE\plugins\EngineFixes_SNCT.ini)");

        if (loadRes < 0)
        {
            _VMESSAGE("unable to load SNCT ini, disabling patch");
            return false;
        }

        _VMESSAGE("hooking vtbls");
        orig_INIPrefSettingCollection_Unlock = *vtbl_INIPrefSettingCollection_Unlock;
        SKSE::SafeWrite64(vtbl_INIPrefSettingCollection_Unlock.GetAddress(), unrestricted_cast<std::uintptr_t>(hk_INIPrefSettingCollection_Unlock));
        _VMESSAGE("success");
        return true;
    }
}

#include "patches.h"
#include "utils.h"

namespace patches
{
    REL::Offset<bool(void*)> orig_INIPrefSettingCollection_Unlock;

    constexpr std::string_view FILE_NAME = "Data/SKSE/Plugins/EngineFixes_SNCT.toml";

    toml::table& get_store()
    {
        static toml::table store;
        return store;
    }

    bool hk_INIPrefSettingCollection_Unlock(void* thisPtr)
    {
        const auto dataHandler = RE::TESDataHandler::GetSingleton();

        if (dataHandler)
        {
            auto& store = get_store();

            for (auto& soundCategory : dataHandler->GetFormArray<RE::BGSSoundCategory>())
            {
                if (soundCategory->IsMenuCategory())
                {
                    auto fullName = soundCategory->GetFullName();
                    fullName = fullName ? fullName : "";
                    _VMESSAGE("processing %s", fullName);
                    _VMESSAGE("menu flag set, saving");
                    auto localFormID = soundCategory->formID & 0x00FFFFFF;
                    // esl
                    if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                    {
                        localFormID = localFormID & 0x00000FFF;
                    }
                    auto srcFile = soundCategory->GetDescriptionOwnerFile();
                    _VMESSAGE("plugin: %s form id: %08X", srcFile->fileName, localFormID);

                    char localFormIDHex[] = "DEADBEEF";
                    sprintf_s(localFormIDHex, std::extent_v<decltype(localFormIDHex)>, "%08X", localFormID);

                    auto table = store.get_as<toml::table>(srcFile->fileName);
                    if (!table)
                    {
                        table =
                            store.insert_or_assign(
                                     srcFile->fileName,
                                     toml::table{})
                                .first->second.as_table();
                    }

                    table->insert_or_assign(localFormIDHex, static_cast<double>(soundCategory->volumeMult));
                }
            }

            std::ofstream file{ FILE_NAME, std::ios_base::out | std::ios_base::trunc };
            if (!file)
            {
                _VMESSAGE("warning: unable to save snct ini");
            }

            file << store;

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
            auto& store = get_store();

            for (auto& soundCategory : dataHandler->GetFormArray<RE::BGSSoundCategory>())
            {
                auto localFormID = soundCategory->formID & 0x00FFFFFF;
                // esl
                if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                {
                    localFormID = localFormID & 0x00000FFF;
                }
                char localFormIDHex[] = "DEADBEEF";
                sprintf_s(localFormIDHex, std::extent_v<decltype(localFormIDHex)>, "%08X", localFormID);

                auto srcFile = soundCategory->GetDescriptionOwnerFile();
                const auto vol = store[srcFile->fileName][localFormIDHex].as_floating_point();

                if (vol)
                {
                    _VMESSAGE("setting volume for formid %08X", soundCategory->formID);
                    const REL::Offset<bool (**)(RE::BSISoundCategory*, float)> SetVolume{ REL::ID(236602), 0x8 * 0x3 };
                    (*SetVolume)(soundCategory, static_cast<float>(vol->get()));
                }
            }
        }
    }

    bool PatchSaveAddedSoundCategories()
    {
        _VMESSAGE("- save added sound categories -");

        try
        {
            auto& store = get_store();
            store = toml::parse_file(FILE_NAME);
        } catch (const std::exception&)
        {}

        _VMESSAGE("hooking vtbls");
        REL::Offset<std::uintptr_t> vtbl{ REL::ID(230546) };  // INIPrefSettingCollection
        orig_INIPrefSettingCollection_Unlock = vtbl.write_vfunc(0x6, hk_INIPrefSettingCollection_Unlock);
        _VMESSAGE("success");
        return true;
    }
}

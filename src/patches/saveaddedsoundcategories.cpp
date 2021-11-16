#include "patches.h"
#include "utils.h"

namespace patches
{
    REL::Relocation<bool(void*)> orig_INIPrefSettingCollection_Unlock;

    constexpr std::string_view FILE_NAME = "Data/SKSE/Plugins/EngineFixes_SNCT.toml"sv;

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
                    logger::trace(FMT_STRING("processing {}"), fullName);
                    logger::trace("menu flag set, saving"sv);
                    auto localFormID = soundCategory->formID & 0x00FFFFFF;
                    // esl
                    if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                    {
                        localFormID = localFormID & 0x00000FFF;
                    }
                    auto srcFile = soundCategory->GetDescriptionOwnerFile();
                    logger::trace(FMT_STRING("plugin: {} form id: {:08X}"), srcFile->fileName, localFormID);

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

            std::ofstream file{ FILE_NAME.data(), std::ios_base::out | std::ios_base::trunc };
            if (!file)
            {
                logger::trace("warning: unable to save snct ini"sv);
            }

            file << store;

            logger::trace("SNCT save: saved sound categories"sv);
        }
        else
        {
            logger::trace("SNCT save: data handler not ready, not saving sound categories for this call"sv);
        }

        return orig_INIPrefSettingCollection_Unlock(thisPtr);
    }

    void LoadVolumes()
    {
        logger::trace("game has loaded, setting volumes"sv);

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
                    logger::trace(FMT_STRING("setting volume for formid {:08X}"), soundCategory->formID);
                    const REL::Relocation<bool (**)(RE::BSISoundCategory*, float)> SetVolume{ REL::ID(236602), 0x8 * 0x3 };
                    (*SetVolume)(soundCategory, static_cast<float>(vol->get()));
                }
            }
        }
    }

    bool PatchSaveAddedSoundCategories()
    {
        logger::trace("- save added sound categories -"sv);

        try
        {
            auto& store = get_store();
            store = toml::parse_file(FILE_NAME);
        } catch (const std::exception&)
        {}

        logger::trace("hooking vtbls"sv);
        REL::Relocation<std::uintptr_t> vtbl{ REL::ID(230546) };  // INIPrefSettingCollection
        orig_INIPrefSettingCollection_Unlock = vtbl.write_vfunc(0x6, hk_INIPrefSettingCollection_Unlock);
        logger::trace("success"sv);
        return true;
    }
}

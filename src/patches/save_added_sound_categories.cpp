#include "save_added_sound_categories.h"

#include <SimpleIni.h>
#include <spdlog/logger.h>

namespace Patches::SaveAddedSoundCategories
{
    namespace detail
    {
        CSimpleIniA& get_store()
        {
            static CSimpleIniA store;
            return store;
        }

        bool INIPrefSettingCollection_Unlock(RE::INIPrefSettingCollection* a_self)
        {
            if (const auto dataHandler = RE::TESDataHandler::GetSingleton())
            {
                auto& store = get_store();

                for (const auto& soundCategory : dataHandler->GetFormArray<RE::BGSSoundCategory>())
                {
                    if (soundCategory->IsMenuCategory())
                    {
                        auto fullName = soundCategory->GetFullName();
                        fullName = fullName ? fullName : "";
                        REX::TRACE("processing {}"sv, fullName);
                        REX::TRACE("menu flag set, saving"sv);
                        auto localFormID = soundCategory->formID & 0x00FFFFFF;
                        // esl
                        if ((soundCategory->formID & 0xFF000000) == 0xFE000000)
                        {
                            localFormID = localFormID & 0x00000FFF;
                        }
                        const auto srcFile = soundCategory->GetDescriptionOwnerFile();
                        REX::TRACE("plugin: {} form id: {:08X} volume: {}"sv, srcFile->fileName, localFormID, soundCategory->volumeMult);

                        char localFormIDHex[] = "DEADBEEF";
                        sprintf_s(localFormIDHex, std::extent_v<decltype(localFormIDHex)>, "%08X", localFormID);

                        store.SetDoubleValue(srcFile->fileName, localFormIDHex, soundCategory->volumeMult);
                    }
                }

               auto ret = store.SaveFile(FILE_NAME.data());

                if (ret < 0)
                {
                    REX::TRACE("warning: unable to save SNCT ini");
                }
                else
                    REX::TRACE("saved SNCT volumes to ini");
            }
            return g_hk_INIPrefSettingCollection_Unlock.call<bool>(a_self);
        }

        void LoadVolumes()
        {
            REX::TRACE("loading SNCT volumes");

            const auto dataHandler = RE::TESDataHandler::GetSingleton();

            if (dataHandler)
            {
                auto& store = get_store();
                auto ret = store.LoadFile(FILE_NAME.data());
                if (ret < 0)
                {
                    REX::ERROR("unable to load SNCT volume ini");
                    return;
                }

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
                    auto vol = store.GetDoubleValue(srcFile->fileName, localFormIDHex, DBL_MAX);

                    if (vol != DBL_MAX)
                    {
                        REX::TRACE("setting volume for formid {:08X} to {}"sv, soundCategory->formID, vol);
                        soundCategory->SetCategoryVolume(static_cast<float>(vol));
                    }
                }
            }
        }
    }

    void LoadVolumes() { detail::LoadVolumes(); }
}
#pragma once

namespace Patches::SaveAddedSoundCategories
{
    namespace detail
    {
        constexpr std::string_view FILE_NAME = "Data/SKSE/Plugins/EngineFixes_SNCT.ini"sv;

        inline SafetyHookInline g_hk_INIPrefSettingCollection_Unlock{};
        bool                    INIPrefSettingCollection_Unlock(RE::INIPrefSettingCollection* a_self);
    }

    void LoadVolumes();

    inline void Install()
    {
        const REL::Relocation unlock{ RELOCATION_ID(74240, 75944) };
        detail::g_hk_INIPrefSettingCollection_Unlock = safetyhook::create_inline(unlock.address(), detail::INIPrefSettingCollection_Unlock);

        logger::info("installed save added sound category volumes patch"sv);
    }
}
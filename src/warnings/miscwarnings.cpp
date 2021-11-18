#include "utils.h"
#include "warnings.h"

namespace warnings
{
    std::unordered_map<std::uint32_t, RE::BGSAddonNode*> nodeMap;

    typedef bool (*_BGSAddonNode_LoadForm)(RE::BGSAddonNode* addonNode, RE::TESFile* modInfo);
    REL::Relocation<_BGSAddonNode_LoadForm*> vtbl_BGSAddonNode_LoadForm{ offsets::DuplicateAddonNodeIndex::BGSAddonNode_vtbl.address() + 0x8 * 0x6 };
    _BGSAddonNode_LoadForm orig_BGSAddonNode_LoadForm;

    bool hk_BGSAddonNode_LoadForm(RE::BGSAddonNode* addonNode, RE::TESFile* modInfo)
    {
        bool retVal = orig_BGSAddonNode_LoadForm(addonNode, modInfo);

        if (retVal)
        {
            //_MESSAGE("form %08x node index %d mod name %s", addonNode->formID, addonNode->nodeIndex, modInfo->name);
            const auto res = nodeMap.insert(std::make_pair(addonNode->index, addonNode));
            if (!res.second)
            {
                const auto current = (*res.first).second;

                if (current != addonNode && current->formID != addonNode->formID)
                {
                    auto srcFile = current->GetDescriptionOwnerFile();
                    logger::warn(FMT_STRING("duplicate addon node index found, formID {:08X} in plugin {} and formID {:08X} in plugin {} share node index {}"), current->formID, srcFile->fileName, addonNode->formID, modInfo->fileName, addonNode->index);
                    logger::warn("for info on resolving this problem, please check the Engine Fixes mod page https://www.nexusmods.com/skyrimspecialedition/mods/17230"sv);
                    logger::warn("you can disable this warning in the ini file"sv);
                    MessageBoxW(nullptr, L"WARNING: You have a duplicate Addon Node index. Please check the Engine Fixes log for more details.", L"Engine Fixes for Skyrim Special Edition", MB_OK);
                }
            }
        }

        return retVal;
    }

    void ClearNodeMap()
    {
        nodeMap.clear();
    }

    // from Ryan
    // Seems to be called when *(Main::Singleton + 0x11) != 0, which is set in ModsChanged_ConfirmResetCallback()
    // I'm not sure if this is the reload data function, but it gets me what I want, which is when the data handler invalidates and reloads all forms
    void Hook_Main_Unk(void* a_this)
    {
        typedef void _Main_Unk_t(void* a_this);
        static REL::Relocation<_Main_Unk_t*> orig_Fn{ offsets::DuplicateAddonNodeIndex::Unk_DataReload_Func };

        orig_Fn(a_this);

        logger::trace("data reload, clearing node map"sv);
        nodeMap.clear();
    }

    bool PatchDupeAddonNodes()
    {
        logger::trace("- warn dupe addon nodes -"sv);
        auto& trampoline = SKSE::GetTrampoline();

        orig_BGSAddonNode_LoadForm = *vtbl_BGSAddonNode_LoadForm;
        REL::safe_write(vtbl_BGSAddonNode_LoadForm.address(), reinterpret_cast<std::uintptr_t>(&hk_BGSAddonNode_LoadForm));

        REL::Relocation<std::uintptr_t> call1_Main_Unk{ offsets::SafeExit::WinMain.address() + 0x1A4 };
        trampoline.write_call<5>(call1_Main_Unk.address(), reinterpret_cast<std::uintptr_t>(&Hook_Main_Unk));

        REL::Relocation<std::uintptr_t> call2_Main_Unk{ offsets::WaterflowAnimation::Main_Update.address() + 0xA2F };
        trampoline.write_call<5>(call2_Main_Unk.address(), reinterpret_cast<std::uintptr_t>(&Hook_Main_Unk));

        logger::trace("- hooked -"sv);
        return true;
    }

    REL::Relocation<std::uint32_t*> g_RefrHandleArray{ offsets::RefrHandleLimit::g_RefrHandleArray };

    void WarnActiveRefrHandleCount(std::uint32_t warnCount)
    {
        const auto refrArray = g_RefrHandleArray.get();

        constexpr std::uint32_t maxHandleCount = 1 << 20;

        std::uint32_t activeHandleCount = 0;

        for (std::uint32_t i = 0; i < maxHandleCount; i++)
        {
            if ((refrArray[i * 4] & (1 << 26)) != 0)
                activeHandleCount++;
        }

        if (activeHandleCount > warnCount)
        {
            logger::warn(FMT_STRING("your active refr handle count is currently {} which is higher than the warning level of {}"), activeHandleCount, warnCount);
            if (warnCount == *config::warnRefrMainMenuLimit)
                logger::warn("this is your main menu limit"sv);
            if (warnCount == *config::warnRefrLoadedGameLimit)
                logger::warn("this is your loaded game limit"sv);
            logger::warn("for info about this warning, please check the Engine Fixes mod page https://www.nexusmods.com/skyrimspecialedition/mods/17230"sv);
            logger::warn("you can disable this warning in the ini file"sv);

            std::wostringstream warningString;
            warningString << L"WARNING: Your active refr handle count is currently "sv << activeHandleCount << L" which is dangerously close to the limit. Please check the Engine Fixes log for more details."sv;
            MessageBoxW(nullptr, warningString.str().c_str(), L"Engine Fixes for Skyrim Special Edition", MB_OK);
        }
    }
}

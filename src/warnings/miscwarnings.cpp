#include "RE/BGSAddonNode.h"
#include "RE/TESFile.h"

#include <sstream>

#include "warnings.h"


namespace warnings
{
    std::unordered_map<uint32_t, RE::BGSAddonNode *> nodeMap;

    typedef bool(*_BGSAddonNode_LoadForm)(RE::BGSAddonNode * addonNode, RE::TESFile * modInfo);
    RelocPtr<_BGSAddonNode_LoadForm> vtbl_BGSAddonNode_LoadForm(vtbl_BGSAddonNode_LoadForm_offset);
    _BGSAddonNode_LoadForm orig_BGSAddonNode_LoadForm;

    RelocPtr<uint32_t> g_RefrHandleArray(g_RefrHandleArray_offset);

    bool hk_BGSAddonNode_LoadForm(RE::BGSAddonNode *addonNode, RE::TESFile * modInfo)
    {
        bool retVal = orig_BGSAddonNode_LoadForm(addonNode, modInfo);

        if (retVal)
        {
            //_MESSAGE("form %08x node index %d mod name %s", addonNode->formID, addonNode->nodeIndex, modInfo->name);
            const auto res = nodeMap.insert(std::make_pair(addonNode->nodeIndex, addonNode));
            if (!res.second)
            {
                const auto current = (*res.first).second;

                if (current != addonNode && current->formID != addonNode->formID)
                {
                    _MESSAGE("WARNING: duplicate addon node index found, formID %08x in plugin %s and formID %08x in plugin %s share node index %d", current->formID, (*current->sourceFiles->files)->name, addonNode->formID, modInfo->name, addonNode->nodeIndex);
                    _MESSAGE("WARNING: for info on resolving this problem, please check the Engine Fixes mod page https://www.nexusmods.com/skyrimspecialedition/mods/17230");
                    _MESSAGE("WARNING: you can disable this warning in the ini file");
                    MessageBox(nullptr, TEXT("WARNING: You have a duplicate Addon Node index. Please check the Engine Fixes log for more details."), TEXT("Engine Fixes for Skyrim Special Edition"), MB_OK);
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
        static RelocAddr<_Main_Unk_t*> orig_Fn(Unk_DataReload_Func_offset);

        orig_Fn(a_this);

        _VMESSAGE("data reload, clearing node map");
        nodeMap.clear();
    }
    
    bool PatchDupeAddonNodes()
    {
        _VMESSAGE("- warn dupe addon nodes -");
        orig_BGSAddonNode_LoadForm = *vtbl_BGSAddonNode_LoadForm;
        SafeWrite64(vtbl_BGSAddonNode_LoadForm.GetUIntPtr(), GetFnAddr(hk_BGSAddonNode_LoadForm));

        RelocAddr<uintptr_t> call1_Main_Unk(Call1_Unk_DataReload_func_offset + 0x163);
        g_branchTrampoline.Write5Call(call1_Main_Unk.GetUIntPtr(), GetFnAddr(&Hook_Main_Unk));

        RelocAddr<uintptr_t> call2_Main_Unk(Call2_Unk_DataReload_func_offset + 0xD);
        g_branchTrampoline.Write5Call(call2_Main_Unk.GetUIntPtr(), GetFnAddr(&Hook_Main_Unk));

        _VMESSAGE("- hooked -");
        return true;
    }

    void WarnActiveRefrHandleCount(uint32_t warnCount)
    {
        const auto refrArray = &*g_RefrHandleArray;

        constexpr uint32_t maxHandleCount = 1 << 20;

        uint32_t activeHandleCount = 0;

        for (auto i = 0; i < maxHandleCount; i++)
        {
            if ((refrArray[i * 4] & (1 << 26)) != 0)
                activeHandleCount++;
        }

        if (activeHandleCount > warnCount)
        {
            _MESSAGE("WARNING: your active refr handle count is currently %d which is higher than the warning level of %d", activeHandleCount, warnCount);
            if (warnCount == config::warnRefrMainMenuLimit)
                _MESSAGE("WARNING: this is your main menu limit");
            if (warnCount == config::warnRefrLoadedGameLimit)
                _MESSAGE("WARNING: this is your loaded game limit");
            _MESSAGE("WARNING: for info about this warning, please check the Engine Fixes mod page https://www.nexusmods.com/skyrimspecialedition/mods/17230");
            _MESSAGE("WARNING: you can disable this warning in the ini file");

            std::ostringstream warningString;
            warningString << "WARNING: Your active refr handle count is currently " << activeHandleCount << " which is dangerously close to the limit. Please check the Engine Fixes log for more details.";
            MessageBox(nullptr, warningString.str().c_str(), TEXT("Engine Fixes for Skyrim Special Edition"), MB_OK);
        }
    }
}

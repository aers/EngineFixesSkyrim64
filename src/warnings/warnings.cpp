#include "warnings.h"

#include <fmt/format.h>

namespace Warnings
{
    void WarnActiveRefrHandleCount(std::uint32_t warnCount)
    {
        const auto refrArray = RE::BSPointerHandleManager<RE::TESObjectREFR*>::GetHandleEntries();

        std::uint32_t activeHandleCount = 0;

        for (auto& val : refrArray) {
            if (val.IsInUse())
                activeHandleCount++;
        }

        if (activeHandleCount > warnCount) {
            REX::WARN(FMT_STRING("your active refr handle count is currently {} which is higher than the warning level of {}"), activeHandleCount, warnCount);
            if (warnCount == Settings::Warnings::uRefrMainMenuLimit.GetValue())
                REX::WARN("this is your main menu limit"sv);
            if (warnCount == Settings::Warnings::uRefrLoadedGameLimit.GetValue())
                REX::WARN("this is your loaded game limit"sv);
            REX::WARN("for info about this warning, please check the Engine Fixes mod page https://www.nexusmods.com/skyrimspecialedition/mods/17230"sv);
            REX::WARN("you can disable this warning in the ini file"sv);

            std::wostringstream warningString;
            warningString << L"WARNING: Your active refr handle count is currently "sv << activeHandleCount << L" which is dangerously close to the limit. Please check the Engine Fixes log for more details."sv;
            REX::W32::MessageBoxW(nullptr, warningString.str().c_str(), L"Engine Fixes for Skyrim Special Edition", MB_OK);
        }
    }
}
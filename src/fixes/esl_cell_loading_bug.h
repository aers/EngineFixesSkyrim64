#pragma once
#include <stacktrace>

namespace Fixes::ESLCELLLoadingBugs
{
    namespace detail
    {
        inline SafetyHookInline orig_GetGroupBlockKey;

        inline std::uint32_t TESObjectCELL_GetGroupBlockKey(const RE::TESObjectCELL* a_this)
        {
            if (a_this->IsInteriorCell()) {
                const RE::TESFile* file = a_this->GetFile(0);
                if (file && file->IsLight())
                    return (a_this->GetFormID() & 0xFFF) % 0xA;
                else
                    return (a_this->GetFormID() & 0xFFFFFF) % 0xA;
            }

            if (a_this->cellData.exterior) {
                return (a_this->cellData.exterior->cellY >> 5) | ((a_this->cellData.exterior->cellX >> 5) << 16);
            }

            return 0;
        }

        inline SafetyHookInline orig_GetGroupSubBlockKey;

        inline std::uint32_t TESObjectCELL_GetGroupSubBlockKey(const RE::TESObjectCELL* a_this)
        {
            if (a_this->IsInteriorCell()) {
                const RE::TESFile* file = a_this->GetFile(0);
                if (file && file->IsLight())
                    return ((a_this->GetFormID() & 0xFFF) % 0x64) / 0xA;
                else
                    return ((a_this->GetFormID() & 0xFFFFFF) % 0x64) / 0xA;
            }

            if (a_this->cellData.exterior) {
                return (a_this->cellData.exterior->cellY >> 3) | ((a_this->cellData.exterior->cellX >> 3) << 16);
            }

            return 0;
        }

        inline void Install()
        {
            orig_GetGroupBlockKey = safetyhook::create_inline(REL::ID(18885).address(), TESObjectCELL_GetGroupBlockKey);
            orig_GetGroupSubBlockKey = safetyhook::create_inline(REL::ID(18886).address(), TESObjectCELL_GetGroupSubBlockKey);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed ESL CELL load bug fix"sv);
    }
}
#pragma once

namespace Fixes::ClimateLoad
{
    namespace detail
    {
        struct Sky
        {
            static void LoadGame(RE::Sky* a_self, RE::BGSLoadGameBuffer* a_loadGameBuffer)
            {
                _LoadGame(a_self, a_loadGameBuffer);

                using Flags = RE::Sky::Flags;
                a_self->flags.set(Flags::kUpdateSunriseBegin, Flags::kUpdateSunriseEnd, Flags::kUpdateSunsetBegin, Flags::kUpdateSunsetEnd, Flags::kUpdateColorsSunriseBegin, Flags::kUpdateColorsSunsetEnd);
            }

            static inline REL::Relocation<decltype(LoadGame)> _LoadGame;
        };
    }

    inline void Install()
    {
        constexpr auto todo = std::array{
            std::make_pair(26218, 0x1A0),
            std::make_pair(35642, 0x241)
        };

        for (const auto& [id, offset] : todo)
        {
            REL::Relocation target { REL::ID(id), offset };
            detail::Sky::_LoadGame = target.write_call<5>(detail::Sky::LoadGame);
        }

        REX::INFO("installed climate load fix"sv);
    }
}
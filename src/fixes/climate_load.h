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
            std::make_pair(25675, 0x124),
            std::make_pair(34736, 0x100)
        };

        for (const auto& [id, offset] : todo)
        {
            REL::Relocation target { REL::ID(id), offset };
            detail::Sky::_LoadGame = target.write_call<5>(detail::Sky::LoadGame);
        }

        logger::info("installed climate load fix"sv);
    }
}
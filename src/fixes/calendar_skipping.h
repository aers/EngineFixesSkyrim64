#pragma once

namespace Fixes::CalendarSkipping
{
    namespace detail
    {
        struct Calendar
        {
            static void Update(RE::Calendar* a_this, float a_seconds)
            {
                _Update(a_this, a_seconds);

                float hoursPassed = (a_seconds * a_this->timeScale->value / (60.0F * 60.0F)) + a_this->gameHour->value - 24.0F;
                if (hoursPassed > 24.0) {
                    do {
                        a_this->midnightsPassed += 1;
                        a_this->rawDaysPassed += 1.0F;
                        hoursPassed -= 24.0F;
                    } while (hoursPassed > 24.0F);
                    a_this->gameDaysPassed->value = (hoursPassed / 24.0F) + a_this->rawDaysPassed;
                }
            }

            static inline REL::Relocation<decltype(Update)> _Update;
        };
    }

    inline void Install()
    {
        // these are all the callsites for ID 36291
#ifdef SKYRIM_AE
        constexpr std::array todo = {
            std::make_pair(13328, 0xE2),
            std::make_pair(36564, 0x266),
            std::make_pair(36566, 0x3A),
            std::make_pair(40445, 0x282),
            std::make_pair(40485, 0x78),
        };
#else
        constexpr std::array todo = {
            std::make_pair(13183, 0xE2),
            std::make_pair(35565, 0x24D),
            std::make_pair(35567, 0x3A),
            std::make_pair(39373, 0x2B1),
            std::make_pair(39410, 0x78),
        };
#endif

        for (const auto& [id, offset] : todo) {
            REL::Relocation target{ REL::ID(id), offset };
            detail::Calendar::_Update = target.write_call<5>(detail::Calendar::Update);
        }

        logger::info("installed calendar skipping fix"sv);
    }
}
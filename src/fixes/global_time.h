#pragma once

namespace Fixes::GlobalTime
{
    inline void Install()
    {
#ifdef SKYRIM_AE
        constexpr std::array todo = {
            std::make_pair(51049, 0xB70 + 0x4), // BookMenu::vf4
            std::make_pair(52486, 0x1BE + 0x4), // SleepWaitMenu::vf4
            std::make_pair(50913, 0x3B + 0x4), // ThirdPersonState::UpdateDelayedParameters
            std::make_pair(50913, 0x9D + 0x4),
            std::make_pair(50913, 0x1B6 + 0x4),
            std::make_pair(50911, 0x264 + 0x4),
            std::make_pair(50921, 0x13 + 0x4)
        };
#else
        constexpr std::array todo = {
            std::make_pair(50118, 0xA91 + 0x4),  // BookMenu::vf4
            std::make_pair(51614, 0x1BC + 0x4),  // SleepWaitMenu::vf4
            std::make_pair(49977, 0x2B + 0x4),   // ThirdPersonState::UpdateDelayedParameters
            std::make_pair(49977, 0x92 + 0x4),
            std::make_pair(49977, 0x1F9 + 0x4),
            std::make_pair(49980, 0xB6 + 0x4),
            std::make_pair(49981, 0x13 + 0x4)
        };
#endif

        for (const auto& [id, offset] : todo)
        {
            REL::Relocation target{ REL::ID(id), offset };
            const REL::Relocation secondsSinceLastFrameRealTime{ RELOCATION_ID(523661, 410200) };
            // offset = loc - addr - bytes til end of instruction
            const auto timerOffset = static_cast<std::int32_t>(secondsSinceLastFrameRealTime.address() - target.address() - 0x4);
            target.write(timerOffset);
        }

        logger::info("installed global time fix"sv);
    }
}
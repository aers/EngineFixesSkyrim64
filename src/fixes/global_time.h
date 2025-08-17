#pragma once

namespace Fixes::GlobalTime
{
    inline void Install()
    {
        constexpr std::array todo = {
            std::make_pair(51049, 0xB70 + 0x4), // BookMenu::vf4
            std::make_pair(52486, 0x1BE + 0x4), // SleepWaitMenu::vf4
            std::make_pair(50913, 0x3B + 0x4), // ThirdPersonState::UpdateDelayedParameters
            std::make_pair(50913, 0x9D + 0x4),
            std::make_pair(50913, 0x1B6 + 0x4),
            std::make_pair(50911, 0x264 + 0x4),
            std::make_pair(50921, 0x13 + 0x4)
        };

        for (const auto& [id, offset] : todo)
        {
            REL::Relocation target{ REL::ID(id), offset };
            const REL::Relocation secondsSinceLastFrameRealTime{ REL::ID(410200) };
            // offset = loc - addr - bytes til end of instruction
            const auto timerOffset = static_cast<std::int32_t>(secondsSinceLastFrameRealTime.address() - target.address() - 0x4);
            target.write(timerOffset);
        }

        REX::INFO("installed global time fix"sv);
    }
}
#pragma once

namespace Fixes::LipSync
{
    inline void Install()
    {
        constexpr std::array offsets{
            0x27,
            0x67,
            0xF4,
            0x126
        };

        REL::Relocation targetBase { RELOCATION_ID(0, 16267) };

        constexpr auto JMP = std::uint8_t { 0xEB };

        for (auto& offset : offsets)
        {
            REL::safe_write(targetBase.address() + offset, JMP);
        }

        logger::info("installed lip sync fix"sv);
    }
}
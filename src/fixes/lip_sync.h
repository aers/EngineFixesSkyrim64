#pragma once

namespace Fixes::LipSync
{
    inline void Install()
    {
        constexpr std::array offsets{
            0x1E,
            0x3A,
            0x9A,
            0xD8
        };

        REL::Relocation targetBase { REL::ID(16023) };

        constexpr auto JMP = std::uint8_t { 0xEB };

        for (auto& offset : offsets)
        {
            REL::safe_write(targetBase.address() + offset, JMP);
        }

        logger::info("installed lip sync fix"sv);
    }
}
#pragma once

namespace Fixes::LipSync
{
    inline void Install()
    {
#ifdef SKYRIM_AE
        constexpr std::array offsets{
            0x27,
            0x67,
            0xF4,
            0x126
        };
#else
        constexpr std::array offsets{
            0x1E,
            0x3A,
            0x9A,
            0xD8
        };
#endif

        REL::Relocation targetBase{ RELOCATION_ID(16023, 16267) };

        constexpr auto JMP = std::uint8_t{ 0xEB };

        for (auto& offset : offsets) {
            REL::WriteSafeData(reinterpret_cast<void*>(targetBase.address() + offset), JMP);
        }

        REX::INFO("installed lip sync fix"sv);
    }
}
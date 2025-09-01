#pragma once

namespace Patches::ScrollingDoesntSwitchPOV
{
    inline void Install()
    {
        REL::Relocation firstPersonState{ RELOCATION_ID(49800, 50730), 0x43 };
        REL::Relocation thirdPersonState{ RELOCATION_ID(49970, 50906), VAR_NUM(0x1E8, 0x1F7) };

        constexpr std::uint8_t BYTE{ 0xEB };
        // replace conditional jump with unconditional jump
        firstPersonState.write(BYTE);
        thirdPersonState.write(BYTE);

        logger::info("installed scrolling doesn't switch pov patch"sv);
    }
}
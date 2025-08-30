#pragma once

namespace Patches::ScrollingDoesntSwitchPOV
{
    inline void Install()
    {
        REL::Relocation firstPersonState { REL::ID(49800), 0x43 };
        REL::Relocation thirdPersonState { REL::ID(49970), 0x1E8 };


        constexpr std::uint8_t BYTE { 0xEB };
        // replace conditional jump with unconditional jump
        firstPersonState.write( BYTE );
        thirdPersonState.write( BYTE );

        logger::info("installed scrolling doesn't switch pov patch"sv);
    }
}
#pragma once

namespace Fixes::AnimationLoadSignedCrash
{
    inline void Install()
    {
        // patch "movsx" to "movzx"
        REL::Relocation target { RELOCATION_ID(64198, 65232), VAR_NUM(0x91, 0xAA) };
        target.write( std::uint8_t { 0xB7 } );

        logger::info("installed animation load signed crash fix"sv);
    }
}
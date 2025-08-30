#pragma once

namespace Fixes::AnimationLoadSignedCrash
{
    inline void Install()
    {
        // patch "movsx" to "movzx"
        REL::Relocation target { REL::ID(64198), 0x91 };
        target.write( std::uint8_t { 0xB7 } );

        logger::info("installed animation load signed crash fix"sv);
    }
}
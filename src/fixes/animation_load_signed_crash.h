#pragma once

namespace Fixes::AnimationLoadSignedCrash
{
    inline void Install()
    {
        // patch "movsx" to "movzx"
        REL::Relocation target { REL::ID(65232), 0xAA };
        target.write( std::uint8_t { 0xB7 } );

        REX::INFO("installed animation load signed crash fix"sv);
    }
}
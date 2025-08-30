#pragma once

namespace Fixes::MO5STypo
{
    inline void Install()
    {
        // change "MODS" to "MO5S"
        REL::Relocation target { REL::ID(14653), 0x83 };
        target.write(uint8_t { 0x35 } );

        logger::info("installed MO5S typo fix"sv);
    }
}
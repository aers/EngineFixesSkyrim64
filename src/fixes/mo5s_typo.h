#pragma once

namespace Fixes::MO5STypo
{
    inline void Install()
    {
        // change "MODS" to "MO5S"
        REL::Relocation target { RELOCATION_ID(0, 14827), 0x8D };
        target.write(uint8_t { 0x35 } );

        logger::info("installed MO5S typo fix"sv);
    }
}
#pragma once

namespace Patches::DisableChargenPrecache
{
    inline void Install()
    {
        REL::Relocation chargenPrecache { RELOCATION_ID(0, 52369) };
        REL::Relocation chargenPrecacheClear { RELOCATION_ID(0, 52381) };

        chargenPrecache.write_fill(REL::INT3, 0x19A);
        chargenPrecache.write(REL::RET);

        chargenPrecacheClear.write_fill(REL::INT3, 0x8A);
        chargenPrecacheClear.write(REL::RET);

        logger::info("installed disable chargen precache patch"sv);

    }
}
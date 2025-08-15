#pragma once

namespace Patches::DisableChargenPrecache
{
    inline void Install()
    {
        REL::Relocation chargenPrecache { REL::ID(52369) };
        REL::Relocation chargenPrecacheClear { REL::ID(52381) };

        chargenPrecache.write_fill(REL::INT3, 0x19A);
        chargenPrecache.write(REL::RET);

        chargenPrecacheClear.write_fill(REL::INT3, 0x8A);
        chargenPrecacheClear.write(REL::RET);

        REX::INFO("installed disable chargen precache patch");

    }
}
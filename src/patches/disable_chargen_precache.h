#pragma once

namespace Patches::DisableChargenPrecache
{
    inline void Install()
    {
        REL::Relocation chargenPrecache{ REL::ID(51509) };
        REL::Relocation chargenPrecacheClear { REL::ID(51507) };

        chargenPrecache.write_fill(REL::INT3, 0xF1);
        chargenPrecache.write(REL::RET);

        chargenPrecacheClear.write_fill(REL::INT3, 0x76);
        chargenPrecacheClear.write(REL::RET);

        logger::info("installed disable chargen precache patch"sv);

    }
}
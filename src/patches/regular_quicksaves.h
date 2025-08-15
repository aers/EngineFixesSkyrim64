#pragma once

namespace Patches::RegularQuicksaves
{
    inline void Install()
    {
        REL::Relocation quickSaveLoadHandlerProcessButtonSaveType{ REL::ID(52251), 0x68 };
        REL::Relocation quickSaveLoadHandlerProcessButtonLoadType{ REL::ID(52251), 0x9B };

        constexpr std::uint32_t regular_save = 0xF0000080;
        constexpr std::uint32_t load_last_save = 0xD0000100;

        quickSaveLoadHandlerProcessButtonSaveType.write(regular_save);
        quickSaveLoadHandlerProcessButtonLoadType.write(load_last_save);

        REX::INFO("installed regular quicksave patch"sv);
    }
}
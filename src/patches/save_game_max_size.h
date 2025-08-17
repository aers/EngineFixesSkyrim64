#pragma once
#include "settings.h"

namespace Patches::SaveGameMaxSize
{
    inline void Install()
    {
        constexpr std::array todo = {
            std::pair(109378, 0x17),
            std::pair(109355, 0x20B),
            std::pair(36095, 0x1)
        };

        if (Settings::Patches::iSaveGameMaxSize.GetValue() > 4095)
        {
            REX::ERROR("iSaveGameMaxSize of {} is too large"sv, Settings::Patches::iSaveGameMaxSize.GetValue());
            return;
        }

        std::uint32_t sizeBytes = Settings::Patches::iSaveGameMaxSize.GetValue() * 1024 * 1024;

        for (auto& [id, offset] : todo)
        {
            REL::Relocation target { REL::ID(id), offset };
            target.write(sizeBytes);
        }

        REX::INFO("installed save game max size patch"sv, Settings::Patches::iSaveGameMaxSize.GetValue());
    }
}
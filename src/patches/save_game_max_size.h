#pragma once
#include "settings.h"

namespace Patches::SaveGameMaxSize
{
    inline void Install()
    {
        if (!Settings::MemoryManager::bOverrideScrapHeap.GetValue()) {
            logger::info("skipping save game max size patch as it requires scrap heap override patch"sv);
            return;
        }
#ifdef SKYRIM_AE
        constexpr std::array todo = {
            std::pair(109378, 0x17),
            std::pair(109355, 0x20B),
            std::pair(36095, 0x1)
        };
#else
        constexpr std::array todo = {
            std::pair(101985, 0x11),
            std::pair(101962, 0x14B),
            std::pair(35203, 0x1)
        };
#endif

        if (Settings::Patches::iSaveGameMaxSize.GetValue() > 4095)
        {
            logger::error("iSaveGameMaxSize of {} is too large"sv, Settings::Patches::iSaveGameMaxSize.GetValue());
            return;
        }

        std::uint32_t sizeBytes = Settings::Patches::iSaveGameMaxSize.GetValue() * 1024 * 1024;

        for (auto& [id, offset] : todo)
        {
            REL::Relocation target { REL::ID(id), offset };
            target.write(sizeBytes);
        }

        logger::info("installed save game max size patch"sv, Settings::Patches::iSaveGameMaxSize.GetValue());
    }
}
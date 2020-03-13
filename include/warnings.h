#pragma once

#include "config.h"
#include "offsets.h"

namespace warnings
{
    bool PatchDupeAddonNodes();
    void ClearNodeMap();
    void WarnActiveRefrHandleCount(uint32_t warnCount);

    bool PatchAll();
}

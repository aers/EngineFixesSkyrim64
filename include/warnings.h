#pragma once

#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/Relocation.h"
#include "skse64_common/SafeWrite.h"

#include "xbyak/xbyak.h"

#include "config.h"
#include "offsets.h"

namespace warnings
{
    bool PatchDupeAddonNodes();
    void ClearNodeMap();
    void WarnActiveRefrHandleCount(uint32_t warnCount);

    bool PatchAll();
}
#include "fixes.h"

#include "memory_access_errors.h"

namespace Fixes
{
    void Load()
    {
        if (Settings::Fixes::bMemoryAccessErrors)
            MemoryAccessErrors::Install();
    }
}
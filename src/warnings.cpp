#include "warnings.h"

namespace warnings
{
    bool PatchAll()
    {
        if (config::warnDupeAddonNodes)
            PatchDupeAddonNodes();

        return true;
    }
}

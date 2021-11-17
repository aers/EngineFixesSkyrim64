#include "fixes.h"

namespace fixes
{
    bool PatchGlobalTime()
    {
        logger::trace("- global time fix -"sv);

        for (const auto& elem : offsets::GlobalTime::todo)
        {
            const auto offset = static_cast<std::int32_t>(offsets::Common::g_SecondsSinceLastFrame_RealTime.address() - (elem.address() + 0x8));
            REL::safe_write(elem.address() + 0x4, offset);
        }

        logger::trace("success"sv);
        return true;
    }
}

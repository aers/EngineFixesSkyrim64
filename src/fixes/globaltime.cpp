#include "fixes.h"

namespace fixes
{
    bool PatchGlobalTime()
    {
        logger::trace("- global time fix -"sv);

        for (const auto& [id, offset] : offsets::GlobalTime::todo)
        {
            const REL::Relocation<std::uintptr_t> target{ REL::ID(id), offset };
            const auto timerOffset = static_cast<std::int32_t>(offsets::Common::g_SecondsSinceLastFrame_RealTime.address() - (target.address() + 0x8));
            REL::safe_write(target.address() + 0x4, timerOffset);
        }

        logger::trace("success"sv);
        return true;
    }
}

#pragma once

namespace Patches::OverrideScaleformAllocator
{
    namespace detail
    {
        void Install();
    }

    inline void Install()
    {
        detail::Install();
        REX::INFO("installed override scaleform allocator patch"sv);
    }
}
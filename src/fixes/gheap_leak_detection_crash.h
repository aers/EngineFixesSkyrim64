#pragma once

namespace Fixes::GHeapLeakDetectionCrash
{
    inline void Install()
    {
        REL::Relocation target { REL::ID(85757), 0x4B };
        target.write_fill(REL::NOP, 0x11);

        logger::info("installed gheap leak detection crash fix"sv);
    }
}
#pragma once

namespace Fixes::GHeapLeakDetectionCrash
{
    inline void Install()
    {
        REL::Relocation target { RELOCATION_ID(0, 87837), 0x4B };
        target.write_fill(REL::NOP, 0x11);

        logger::info("installed gheap leak detection crash fix"sv);
    }
}
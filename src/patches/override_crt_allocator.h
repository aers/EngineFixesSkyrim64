#pragma once

namespace Patches::OverrideCRTAllocator {
    inline void Install() {
        REL::PatchIAT(mi_calloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "calloc");
        REL::PatchIAT(mi_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "free");
        REL::PatchIAT(mi_malloc, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "malloc");
        REL::PatchIAT(mi_usable_size, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_msize");
        REL::PatchIAT(mi_free, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_free");
        REL::PatchIAT(mi_malloc_aligned, "API-MS-WIN-CRT-HEAP-L1-1-0.DLL", "_aligned_malloc");

        REX::INFO("installed CRT allocator override patch"sv);
    }
}
#pragma once

namespace Fixes::InitializeHitDataNullPtrCrash
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(uintptr_t a_target)
            {
                Xbyak::Label out;
                // At the point where this is injected we're still moving function parameters into the correct registers.
                // The function uses weapon as rdi, but it's passed to the function in r9.
                // So we clear rdi and only move r9 there if it's safe to do so.
                xor_(rdi, rdi);
                test(r9, r9);
                jz(out);
                mov(rbx, qword[r9]); // rbx is free to clobber at this point
                test(rbx, rbx);
                cmovnz(rdi, r9); // keep weapon only if weapon->object != NULL
                L(out);
                mov(rbp, r8); // Restore this from where the trampoline jump was placed
                jmp(ptr[rip]);
                dq(a_target + 0x6);
            }
        };
    }

    inline void Install()
    {
        REL::Relocation target { RELOCATION_ID(0, 44001), 0x10 };

        detail::Patch p(target.address());
        p.ready();

        auto& trampoline = SKSE::GetTrampoline();
        target.write_branch<6>(trampoline.allocate(p));

        logger::info("installed initialize hit data nullptr crash"sv);
    }
}
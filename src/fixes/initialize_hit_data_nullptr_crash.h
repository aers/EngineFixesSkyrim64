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
#ifdef SKYRIM_AE
                xor_(rdi, rdi);
#else
                xor_(r15, r15);
#endif
                test(r9, r9);
                jz(out);
                mov(rbx, qword[r9]); // rbx is free to clobber at this point
                test(rbx, rbx);
#ifdef SKYRIM_AE
                cmovnz(rdi, r9); // keep weapon only if weapon->object != NULL
#else
                cmovnz(r15, r9);
#endif
                L(out);
                mov(rbp, r8); // Restore this from where the trampoline jump was placed
                jmp(ptr[rip]);
                dq(a_target + 0x6);
            }
        };
    }

    inline void Install()
    {
        REL::Relocation target { RELOCATION_ID(44001, 44001), VAR_NUM(0xE, 0x10) };

        detail::Patch p(target.address());
        p.ready();

        auto& trampoline = SKSE::GetTrampoline();
        target.write_branch<6>(trampoline.allocate(p));

        logger::info("installed initialize hit data nullptr crash"sv);
    }
}
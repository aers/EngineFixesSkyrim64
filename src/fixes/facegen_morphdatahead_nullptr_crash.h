#pragma once

namespace Fixes::FaceGenMorphDataHeadNullPtrCrash
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            Patch(std::uintptr_t a_target, std::uintptr_t a_constant)
            {
                Xbyak::Label zeroLbl;

                // new nullcheck
                test(rbx, rbx);
                jz(zeroLbl);

                // original code
                mov(eax, ptr[rbx+ 0x24]);
                push(rbx);
                mov (rbx, a_constant);
                comiss(xmm6, ptr[rbx]);
                pop(rbx);

                jmp(ptr[rip]);
                dq(a_target + 0xA); // return to regular execution

                L(zeroLbl);
                jmp(ptr[rip]);
                dq(a_target + 0x3F3); // skip code
            }
        };

        struct PatchClearRbx final : Xbyak::CodeGenerator
        {
            PatchClearRbx()
            {
                xor_(ebx, ebx); // 2 bytes
                nop();          // 1 byte
            }
        };
    }

    inline void Install()
    {
        // fix null ptr
        REL::Relocation target { REL::ID(26343), 0x4C };
        REL::Relocation constant { REL::ID(228611) };

        detail::Patch p(target.address(), constant.address());
        p.ready();

        auto& trampoline = SKSE::GetTrampoline();
        target.write_branch<5>(trampoline.allocate(p));

        REL::safe_fill(target.address() + 0x5, REL::NOP, 0x5);

        // fix clearing rbx
        REL::Relocation targetRbx { REL::ID(26343), 0x49 };
        detail::PatchClearRbx pRbx;
        pRbx.ready();

        targetRbx.write(std::span {pRbx.getCode<const std::byte*>(), pRbx.getSize()});

        logger::info("installed facegen morphdatahead nullptr crash fix"sv);
    }
}
#pragma once

namespace Fixes::NiTimeControllerNoTarget
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            Patch(const std::uintptr_t a_target, const std::uintptr_t a_function)
            {
                Xbyak::Label jmpOverSet;
                Xbyak::Label jmpNone;

#ifdef SKYRIM_AE
                mov(rcx, r15);  // NiObjectNET*
                mov(rdx, rbp);  // NiControllerSequence::IDTag*
#else
                mov(rcx, r13);
                mov(rdx, r15);
#endif
                mov(rax, a_function);
                sub(rsp, 0x20);
                call(rax);
                add(rsp, 0x20);
                test(rax, rax);
                jz(jmpOverSet);
#ifdef SKYRIM_AE
                mov(rax, ptr[r13 + 0x20]);
                mov(rdi, ptr[r12 + rax + 0x8]);
                test(rdi, rdi);
#else
                mov(rax, ptr[r12 + 0x20]);
                mov(rbx, ptr[rax + rbp + 0x8]);
                test(rbx, rbx);
#endif
                jz(jmpNone);
                jmp(ptr[rip]);
                dq(a_target + VAR_NUM(0x13, 0x12));

                L(jmpOverSet);
#ifdef SKYRIM_AE
                mov(rax, ptr[r13 + 0x20]);
                mov(rdi, ptr[r12 + rax + 0x8]);
                test(rdi, rdi);
#else
                mov(rax, ptr[r12 + 0x20]);
                mov(rbx, ptr[rax + rbp + 0x8]);
                test(rbx, rbx);
#endif
                jz(jmpNone);
                jmp(ptr[rip]);
                dq(a_target + VAR_NUM(0x61, 0x5B));

                L(jmpNone);
                jmp(ptr[rip]);
                dq(a_target + VAR_NUM(0x177, 0x150));
            }
        };

        inline bool ShouldProcess(const RE::NiObjectNET* a_object, const RE::NiControllerSequence::IDTag* a_tag)
        {
            return a_object->GetRTTI()->GetName() == a_tag->propertyType;
        }

        inline SafetyHookInline orig_LinkObject;

        inline void LinkObject(RE::NiTimeController* a_self, RE::NiStream* a_stream)
        {
            orig_LinkObject.call(a_self, a_stream);
            if (a_self->target == nullptr) {
                logger::warn("{} - NiTimeController of type {} loaded with no target", a_stream->inputFilePath, a_self->GetRTTI()->GetName());
            }
        }

        inline void Install()
        {
            REL::Relocation timeControllerLinkObject { RELOCATION_ID(69434, 70811) };
            orig_LinkObject = safetyhook::create_inline(timeControllerLinkObject.address(), LinkObject);

            REL::Relocation target{ RELOCATION_ID(70880, 72461), VAR_NUM(0x2F0, 0x2E0) };
            auto&           trampoline = SKSE::GetTrampoline();

            Patch p(target.address(), SKSE::stl::unrestricted_cast<std::uintptr_t>(ShouldProcess));
            p.ready();
            target.write_branch<5>(trampoline.allocate(p));
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("nstalled NiTimeController with no target crash fix"sv);
    }
}
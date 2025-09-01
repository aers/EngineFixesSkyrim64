#pragma once

namespace Fixes::TorchLandscape
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(std::uintptr_t a_func)
            {
                Xbyak::Label f;
#ifdef SKYRIM_AE
                mov(r9, rsi);
#else
                mov(r9, rdi);
#endif
                jmp(ptr[rip + f]);

                L(f);
                dq(a_func);
            }
        };

        struct ShadowSceneNode
        {
            static RE::BSLight* AddLight(
                RE::ShadowSceneNode*                      a_this,
                RE::NiLight*                              a_list,
                RE::ShadowSceneNode::LIGHT_CREATE_PARAMS& a_params,
                const RE::TESObjectREFR*                  a_requester)
            {
                if (a_requester->Is(RE::FormType::ActorCharacter)) {
                    a_params.affectLand = true;
                }

                return _AddLight(a_this, a_list, a_params);
            }

            static inline REL::Relocation<RE::BSLight*(RE::ShadowSceneNode*, RE::NiLight*, const RE::ShadowSceneNode::LIGHT_CREATE_PARAMS&)> _AddLight;
        };
    }

    inline void Install()
    {
        REL::Relocation target{ RELOCATION_ID(17208, 17610), VAR_NUM(0x52D, 0x530) };

        detail::Patch p(SKSE::stl::unrestricted_cast<std::uintptr_t>(detail::ShadowSceneNode::AddLight));
        p.ready();

        auto& trampoline = SKSE::GetTrampoline();
        detail::ShadowSceneNode::_AddLight = target.write_call<5>(trampoline.allocate(p));

        logger::info("installed torch landscape fix"sv);
    }
}
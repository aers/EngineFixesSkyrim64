#pragma once

namespace Fixes::WeaponBlockScaling
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            explicit Patch(std::uintptr_t a_target)
            {
                // rbx = Actor*

                mov(rcx, rbx);
                mov(rdx, a_target);
                call(rdx);
                movaps(xmm8, xmm0);
            }
        };

        struct Actor
        {
            static float CalcWeaponDamage(RE::Actor* a_target)
            {
                auto weap = GetWeaponData(a_target);
                if (weap)
                    return static_cast<float>(weap->GetAttackDamage());
                else
                    return 0.0F;
            }

            static RE::TESObjectWEAP* GetWeaponData(RE::Actor* a_actor)
            {
                const auto proc = a_actor->currentProcess;
                if (!proc || !proc->middleHigh)
                {
                    return nullptr;
                }

                const auto middleProc = proc->middleHigh;
                const std::array entries{
                    middleProc->bothHands,
                    middleProc->rightHand,
                    middleProc->leftHand
                };

                for (const auto& entry : entries)
                {
                    if (entry)
                    {
                        const auto obj = entry->GetObject();
                        if (obj && obj->Is(RE::FormType::Weapon))
                        {
                            return static_cast<RE::TESObjectWEAP*>(obj);
                        }
                    }
                }

                return nullptr;
            }
        };
    }

    inline void Install()
    {
        REL::Relocation target {REL::ID(42842), 0x3BB};

        detail::Patch p(SKSE::stl::unrestricted_cast<std::uintptr_t>(detail::Actor::CalcWeaponDamage));
        p.ready();

        target.write(std::span { p.getCode<const std::byte*>(), p.getSize()} );

        REL::safe_fill(target.address() + p.getSize(), REL::NOP, 0x17 - p.getSize());

        logger::info("installed weapon block scaling fix"sv);
    }
}

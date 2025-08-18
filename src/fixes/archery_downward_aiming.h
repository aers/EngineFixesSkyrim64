#pragma once

namespace Fixes::ArcheryDownwardAiming
{
    namespace detail
    {
        struct Projectile
        {
            static void Move(RE::Projectile* a_self, RE::NiPoint3& a_from, const RE::NiPoint3& a_to)
            {
                const auto refShooter = a_self->shooter.get();
                if (refShooter && refShooter->Is(RE::FormType::ActorCharacter))
                {
                    const auto akShooter = static_cast<RE::Actor*>(refShooter.get()); // NOLINT(*-pro-type-static-cast-downcast)
                    [[maybe_unused]] RE::NiPoint3 direction;
                    akShooter->GetEyeVector(a_from, direction, true);
                }

                _Move(a_self, a_from, a_to);
            }

            static inline REL::Relocation<decltype(Move)> _Move;
        };
    }

    inline void Install()
    {
        REL::Relocation target { REL::ID(44027), 0x434 };
        detail::Projectile::_Move = target.write_call<5>(detail::Projectile::Move);

        logger::info("installed archery downward aiming fix"sv);
    }
}
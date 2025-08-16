#pragma once

namespace Fixes::DoublePerkApply
{
    namespace detail
    {
        inline void ApplyPerk(RE::BGSPerk* a_perk, RE::Actor* a_perkOwner, std::uint8_t a_removeRank, std::uint8_t a_applyRank)
        {
            using func_t = decltype(&ApplyPerk);
            const REL::Relocation<func_t> func{ REL::ID(23822) };
            return func(a_perk, a_perkOwner, a_removeRank, a_applyRank);
        }

        inline void RemoveBasePerks(RE::Actor* a_actor)
        {
            using func_t = decltype(&RemoveBasePerks);
            const REL::Relocation<func_t> func{ REL::ID(37704) };
            return func(a_actor);
        }

        inline void ApplyBasePerksActorImplementation(RE::Actor* a_actor)
        {
            RemoveBasePerks(a_actor);

            auto* currentProcess = a_actor->currentProcess;
            if (!currentProcess)
            {
                return;
            }

            auto* middleHighProcessData = currentProcess->middleHigh;
            if (!middleHighProcessData)
            {
                return;
            }

            auto* npc = a_actor->GetActorBase();
            if (!npc)
            {
                return;
            }

            const auto* perkRankArray = static_cast<RE::BGSPerkRankArray*>(npc);

            for (std::uint32_t i = 0; i < perkRankArray->perkCount; i++)
            {
                auto* perk = perkRankArray->perks[i].perk;
                if (!perk)
                {
                    continue;
                }

                ApplyPerk(perk, a_actor, 0, perkRankArray->perks[i].currentRank);
            }
        }

        inline void ApplyBasePerksActor(RE::Actor* a_actor)
        {
            RE::ActorHandle actorHandle{ a_actor };

            SKSE::GetTaskInterface()->AddTask(
                [actorHandle]() {
                    auto* actor = actorHandle.get().get();

                    if (!actor)
                    {
                        return;
                    }

                    ApplyBasePerksActorImplementation(actor);
                });
        }

        inline void ApplyBasePerksPlayerCharacterImplementation(RE::PlayerCharacter* a_player)
        {
            ApplyBasePerksActorImplementation(a_player);

            for (auto* addedPerkRank : a_player->addedPerks)
            {
                auto* perk = addedPerkRank->perk;

                if (!perk)
                {
                    continue;
                }

                ApplyPerk(perk, a_player, 0, addedPerkRank->currentRank);
            }
        }

        inline void ApplyBasePerksPlayerCharacter(RE::PlayerCharacter* a_player)
        {
            RE::BSPointerHandle<RE::PlayerCharacter> playerHandle{ a_player };

            SKSE::GetTaskInterface()->AddTask(
                [playerHandle]() {
                    auto* player = playerHandle.get().get();

                    if (!player)
                    {
                        return;
                    }

                    ApplyBasePerksPlayerCharacterImplementation(player);
                });
        }
    }

    inline void Install()
    {
        REL::Relocation actorVtbl { RE::Actor::VTABLE[0] };
        REL::Relocation characterVtbl { RE::Character::VTABLE[0] };
        REL::Relocation playerCharacterVtbl { RE::PlayerCharacter::VTABLE[0] };

        actorVtbl.write_vfunc(0x101, detail::ApplyBasePerksActor);
        characterVtbl.write_vfunc(0x101, detail::ApplyBasePerksActor);
        playerCharacterVtbl.write_vfunc(0x101, detail::ApplyBasePerksPlayerCharacter);

        REX::INFO("installed double perk apply fix"sv);
    }
}
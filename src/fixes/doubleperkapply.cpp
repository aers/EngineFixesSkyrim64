#include "fixes.h"

namespace fixes
{
    void ApplyPerk(RE::BGSPerk* a_perk, RE::Actor* a_perkOwner, std::uint8_t a_removeRank, std::uint8_t a_applyRank)
    {
        using func_t = decltype(&ApplyPerk);
        REL::Relocation<func_t> func{ REL::ID(23353) };
        return func(a_perk, a_perkOwner, a_removeRank, a_applyRank);
    }

    void RemoveBasePerks(RE::Actor* a_actor)
    {
        using func_t = decltype(&RemoveBasePerks);
        REL::Relocation<func_t> func{ REL::ID(36695) };
        return func(a_actor);
    }

    REL::Relocation<std::uintptr_t> Actor_vtbl{ REL::ID(260538) };
    REL::Relocation<std::uintptr_t> Character_vtbl{ REL::ID(261397) };
    REL::Relocation<std::uintptr_t> PlayerCharacter_vtbl{ REL::ID(261916) };

    void ApplyBasePerksActorImplementation(RE::Actor* a_actor)
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

        auto* npc = static_cast<RE::TESNPC*>(a_actor->GetActorBase());
        if (!npc)
        {
            return;
        }

        auto* perkRankArray = static_cast<RE::BGSPerkRankArray*>(npc);
        if (perkRankArray)
        {
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
    }

    void ApplyBasePerksActor(RE::Actor* a_actor)
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

    void ApplyBasePerksPlayerCharacterImplementation(RE::PlayerCharacter* a_player)
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

    void ApplyBasePerksPlayerCharacter(RE::PlayerCharacter* a_player)
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

    bool PatchDoublePerkApply()
    {
        logger::trace("- double perk apply -"sv);

        logger::trace("patching vtables"sv);

        Actor_vtbl.write_vfunc(0x101, reinterpret_cast<std::uintptr_t>(std::addressof(ApplyBasePerksActor)));
        Character_vtbl.write_vfunc(0x101, reinterpret_cast<std::uintptr_t>(std::addressof(ApplyBasePerksActor)));
        PlayerCharacter_vtbl.write_vfunc(0x101, reinterpret_cast<std::uintptr_t>(std::addressof(ApplyBasePerksPlayerCharacter)));

        logger::trace("success");

        return true;
    }
}

#pragma once

namespace Fixes::RemovedSpellBook
{
    namespace detail
    {
        struct TESObjectBOOK
        {
            static void LoadGame(RE::TESObjectBOOK* a_this, RE::BGSLoadFormBuffer* a_buf)
            {
                using Flag = RE::OBJ_BOOK::Flag;

                _LoadGame(a_this, a_buf);

                if (a_this->data.teaches.actorValueToAdvance == RE::ActorValue::kNone)
                {
                    if (a_this->TeachesSkill())
                    {
                        a_this->data.flags.reset(Flag::kAdvancesActorValue);
                    }

                    if (a_this->TeachesSpell())
                    {
                        a_this->data.flags.reset(Flag::kTeachesSpell);
                    }
                }
            }

            static inline REL::Relocation<decltype(&RE::TESObjectBOOK::LoadGame)> _LoadGame;
        };
    }
    inline void Install()
    {
        REL::Relocation vtbl { RE::TESObjectBOOK::VTABLE[0] };
        detail::TESObjectBOOK::_LoadGame = vtbl.write_vfunc(0xF, detail::TESObjectBOOK::LoadGame);

        REX::INFO("installed removed spell book fix"sv);
    }
}
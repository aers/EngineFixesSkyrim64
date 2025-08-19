#pragma once

namespace Fixes::ConjurationEnchantAbsorbs
{
    namespace detail
    {
        struct EnchantmentItem
        {
            static bool GetNoAbsorb(RE::EnchantmentItem* a_this)
            {
                using Archetype = RE::EffectArchetypes::ArchetypeID;
                for (const auto& effect : a_this->effects)
                {
                    if (effect->baseEffect->HasArchetype(Archetype::kSummonCreature))
                    {
                        return true;
                    }
                }
                return _GetNoAbsorb(a_this);
            }

            static inline REL::Relocation<decltype(&RE::EnchantmentItem::GetNoAbsorb)> _GetNoAbsorb;
        };
    }

    inline void Install()
    {
        REL::Relocation targetVtbl { RE::EnchantmentItem::VTABLE[0] };
        detail::EnchantmentItem::_GetNoAbsorb = targetVtbl.write_vfunc(0x5E, detail::EnchantmentItem::GetNoAbsorb);
    }
}
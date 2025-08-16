#pragma once

namespace Fixes::BSTempEffectNiRTTI
{
    inline void Install()
    {
        const REL::Relocation<RE::NiRTTI*> rttiBSTempEffect{ RE::BSTempEffect::Ni_RTTI };
        const REL::Relocation<RE::NiRTTI*> rttiNiObject{ RE::NiObject::Ni_RTTI };
        rttiBSTempEffect->baseRTTI = rttiNiObject.get();

        REX::INFO("installed bstempeffect nirtti fix"sv);
    }
}
#pragma once

namespace Patches::WaterflowAnimation
{
    namespace detail
    {
        inline REL::Relocation<uint32_t*> g_ApplicationRunTime{ REL::ID(410201) };

        inline float g_Timer = 8 * 3600;  // Game timer inits to 8 AM

        inline void update_timer()
        {
            g_Timer = g_Timer + RE::GetSecondsSinceLastFrame() * Settings::Patches::fWaterflowSpeed.GetValue();
            if (g_Timer > 86400)  // reset timer to 0 if we go past 24 hours
                g_Timer = g_Timer - 86400;
        }

        void PatchWaterflowAnimation();
    }

    inline void Install()
    {
        detail::PatchWaterflowAnimation();

        REX::INFO("installed waterflow animation patch"sv, Settings::Patches::fWaterflowSpeed.GetValue());
    }
}
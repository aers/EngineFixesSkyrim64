#pragma once

namespace Fixes::BSLightingShaderForceAlphaTest
{
    namespace detail
    {
        inline SafetyHookInline g_hk_BSBatchRenderer_SetupAndDrawPass;

        // resolved once at install; this hook runs for every render pass, so an Address Library lookup per call adds up
        inline std::uintptr_t g_BSLightingShaderVtbl = 0;

        // constants
        inline std::uint32_t RAW_FLAG_RIM_LIGHTING = 1 << 11;
        inline std::uint32_t RAW_FLAG_DO_ALPHA_TEST = 1 << 20;
        inline std::uint32_t RAW_TECHNIQUE_EYE = 16;
        inline std::uint32_t RAW_TECHNIQUE_MULTILAYERPARALLAX = 11;
        inline std::uint32_t RAW_TECHNIQUE_ENVMAP = 1;
        inline std::uint32_t RAW_TECHNIQUE_PARALLAX = 3;

        inline void BSBatchRenderer_SetupAndDrawPass(RE::BSRenderPass* a_self, std::uint32_t a_technique, bool a_alphaTest, std::uint32_t a_renderFlags)
        {
            if (a_alphaTest && *REX::UNRESTRICTED_CAST<std::uintptr_t*>(a_self->shader) == g_BSLightingShaderVtbl) {
                const auto rawTechnique = a_technique - 0x4800002D;
                const auto subIndex = (rawTechnique >> 24) & 0x3F;
                if (subIndex != RAW_TECHNIQUE_EYE && subIndex != RAW_TECHNIQUE_ENVMAP && subIndex != RAW_TECHNIQUE_MULTILAYERPARALLAX && subIndex != RAW_TECHNIQUE_PARALLAX) {
                    a_technique = a_technique | RAW_FLAG_DO_ALPHA_TEST;
                    a_self->passEnum = a_technique;
                }
            }

            // the hook is never removed, so the hook mutex that call() takes would only serialize every draw for nothing
            g_hk_BSBatchRenderer_SetupAndDrawPass.unsafe_call(a_self, a_technique, a_alphaTest, a_renderFlags);
        }
    }

    inline void Install()
    {
        detail::g_BSLightingShaderVtbl = RE::VTABLE_BSLightingShader[0].address();

        REL::Relocation target{ RELOCATION_ID(100854, 107644) };
        detail::g_hk_BSBatchRenderer_SetupAndDrawPass = safetyhook::create_inline(target.address(), detail::BSBatchRenderer_SetupAndDrawPass);

        REX::INFO("installed bslightingshader force alpha test fix"sv);
    }
}
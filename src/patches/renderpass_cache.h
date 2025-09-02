#pragma once
#include "allocator/allocator.h"

namespace RenderPassCache
{
    namespace detail
    {
        inline void SetLights(RE::BSRenderPass* a_renderPass, uint8_t a_numLights, RE::BSLight** a_lights)
        {
            if (a_numLights != a_renderPass->numLights) {
                if (a_renderPass->sceneLights) {
                    Allocator::GetAllocator()->DeallocateAligned(a_renderPass->sceneLights);
                    a_renderPass->sceneLights = nullptr;
                }
                if (a_numLights != 0) {
                    a_renderPass->sceneLights = static_cast<RE::BSLight**>(Allocator::GetAllocator()->AllocateAligned(sizeof(RE::BSLight*) * a_numLights, 8));
                }
                a_renderPass->numLights = a_numLights;
            }

            for (uint32_t i = 0; i < a_numLights; i++)
                a_renderPass->sceneLights[i] = a_lights[i];
        }

#ifndef SKYRIM_AE
        inline void Set(RE::BSRenderPass* a_renderPass, RE::BSShader* a_shader, RE::BSShaderProperty* a_property, RE::BSGeometry* a_geometry, uint32_t a_passEnum, uint8_t a_numLights, RE::BSLight** a_lights)
        {
            a_renderPass->shader = a_shader;
            a_renderPass->shaderProperty = a_property;
            a_renderPass->geometry = a_geometry;
            a_renderPass->passEnum = a_passEnum;
            a_renderPass->accumulationHint = 0;
            SetLights(a_renderPass, a_numLights, a_lights);
        }
#endif

        inline RE::BSRenderPass* Allocate(RE::BSShader* a_shader, RE::BSShaderProperty* a_property, RE::BSGeometry* a_geometry, uint32_t a_passEnum, uint8_t a_numLights, RE::BSLight** a_lights)
        {
            constexpr std::size_t size = sizeof(RE::BSRenderPass);
            auto*                 data = Allocator::GetAllocator()->AllocateAligned(size, 8);
            memset(data, 0, size);

            auto* renderPass = static_cast<RE::BSRenderPass*>(data);

            renderPass->shader = a_shader;
            renderPass->shaderProperty = a_property;
            renderPass->geometry = a_geometry;
            renderPass->passEnum = a_passEnum;
            renderPass->accumulationHint = 0;
            renderPass->extraParam = 0;
            renderPass->LODMode.index = 3;
            renderPass->LODMode.singleLevel = false;
            renderPass->unk20 = 0;
            renderPass->next = nullptr;
            renderPass->passGroupNext = nullptr;
            renderPass->cachePoolId = 0xFEFEDEAD;

            SetLights(renderPass, a_numLights, a_lights);

            return renderPass;
        }

        inline void Deallocate(RE::BSRenderPass* a_renderPass)
        {
            if (a_renderPass->sceneLights != nullptr)
                Allocator::GetAllocator()->DeallocateAligned(a_renderPass->sceneLights);
            Allocator::GetAllocator()->DeallocateAligned(a_renderPass);
        }

        inline void Install()
        {
            REL::Relocation allocate{ RELOCATION_ID(100717, 107497) };
            REL::Relocation deallocate{ RELOCATION_ID(100718, 107498) };
            REL::Relocation setlights{ RELOCATION_ID(100711, 107490) };
#ifndef SKYRIM_AE
            REL::Relocation set{ REL::ID(100710) };
#endif
            REL::Relocation init{ RELOCATION_ID(100720, 107500) };
            REL::Relocation kill{ RELOCATION_ID(100721, 107501) };
            REL::Relocation clear{ RELOCATION_ID(100722, 107502) };

            allocate.replace_func(VAR_NUM(0x9A, 0xF9), Allocate);
            deallocate.replace_func(VAR_NUM(0x60, 0x68), Deallocate);
            setlights.replace_func(0x69, SetLights);
#ifndef SKYRIM_AE
            set.replace_func(0x90, Set);
#endif

            init.write_fill(REL::INT3, VAR_NUM(0x1BD, 0x1BF));
            init.write(REL::RET);
            kill.write_fill(REL::INT3, 0xAB);
            kill.write(REL::RET);
            clear.write_fill(REL::INT3, VAR_NUM(0xE7, 0x16B));
            clear.write(REL::RET);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed render pass cache patch"sv);
    }
}
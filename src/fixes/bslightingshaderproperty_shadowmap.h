#pragma once
#include "allocator/allocator.h"

namespace BSLightingShaderPropertyShadowMap
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            Patch(const std::uintptr_t a_target, const std::uintptr_t a_function)
            {
                mov(rcx, rsi);                       // BSLightingShaderProperty*
                mov(rdx, qword[rsp + 0xE8 + 0x10]);  // BSGeometry*
                mov(r8, a_function);
                push(rbx);
                sub(rsp, 0x20);
                call(r8);
                add(rsp, 0x20);
                pop(rbx);
                mov(r15, rax);
                jmp(ptr[rip]);
                dq(a_target + VAR_NUM(0x7EB, 0x802));
            }
        };

        inline std::uint32_t g_currentIndex = 0;

        inline REL::Relocation<RE::BSRenderPass*(RE::BSShader*, RE::BSShaderProperty*, RE::BSGeometry*, std::uint32_t, std::uint8_t, RE::BSLight**)> BSRenderPass_Allocate{ RELOCATION_ID(100717, 107497) };
        inline REL::Relocation<void(RE::BSRenderPass*)>                                                                                              BSRenderPass_Deallocate{ RELOCATION_ID(100718, 107498) };

        inline RE::BSRenderPass** BSLightingShaderProperty_GetRenderPasses_ShadowMapOrMask_Detour(RE::BSLightingShaderProperty* a_property, RE::BSGeometry* a_geometry)
        {
            // create our storage, 4 max
            // re-use the RenderPassArray space here
            if (a_property->unk0D8.unk08 != 0xDEADBEEF) {
                a_property->unk0D8.head = static_cast<RE::BSRenderPass*>(Allocator::GetAllocator()->AllocateAligned(sizeof(RE::BSRenderPass*) * 4, 8));
                memset(a_property->unk0D8.head, 0, sizeof(RE::BSRenderPass*) * 4);
                a_property->unk0D8.unk08 = 0xDEADBEEF;
            }
            auto** passArray = reinterpret_cast<RE::BSRenderPass**>(a_property->unk0D8.head);
            // clear last frame's render pass
            if (passArray[g_currentIndex] != nullptr) {
                BSRenderPass_Deallocate(passArray[g_currentIndex]);
                passArray[g_currentIndex] = nullptr;
            }

            // create new one
            std::uint32_t technique = a_property->DetermineUtilityShaderDecl() | 0xC000;
            const auto*   alphaProperty = reinterpret_cast<RE::NiAlphaProperty*>(a_geometry->properties[0].get());
            if (alphaProperty && (alphaProperty->alphaFlags & 0x200) != 0) {
                technique |= 0x80;
            }
            if (a_property->flags.all(RE::BSShaderProperty::EShaderPropertyFlag::kLODObjects) || a_property->flags.all(RE::BSShaderProperty::EShaderPropertyFlag::kHDLODObjects))
                technique |= 0x8000000;

            RE::BSRenderPass* pass = BSRenderPass_Allocate(RE::BSUtilityShader::GetSingleton(), a_property, a_geometry, technique + 0x2B, 0, nullptr);
            pass->accumulationHint = 8;
            if ((a_geometry->flags.underlying() & 0x8000000) != 0) {
                pass->LODMode.index = a_property->fadeNode->unk152 & 0xF;
            } else {
                pass->LODMode.index = 3;
            }
            pass->LODMode.singleLevel = false;
            passArray[g_currentIndex] = pass;
            return &passArray[g_currentIndex];
        }

        inline SafetyHookInline orig_BSShadowDirectionalLight_14F0920;

        inline void BSShadowDirectionalLight_14F0920(RE::BSShadowLight* a_self, RE::BSShadowLight::ShadowMapData* a_data, std::uint32_t* a_unk3, std::uintptr_t a_unk4)
        {
            // store the currently being processed light's shadowmap index
            // this is singlethreaded so it is safe
            g_currentIndex = a_data->shadowMapIndex;
            orig_BSShadowDirectionalLight_14F0920.call(a_self, a_data, a_unk3, a_unk4);
        }

        inline void CleanAllocatedArrays(RE::BSLightingShaderProperty* a_self)
        {
            if (a_self->unk0D8.unk08 == 0xDEADBEEF) {
                auto** passArray = reinterpret_cast<RE::BSRenderPass**>(a_self->unk0D8.head);
                for (int i = 0; i < 4; i++) {
                    if (passArray[i] != nullptr) {
                        BSRenderPass_Deallocate(passArray[i]);
                        passArray[i] = nullptr;
                    }
                }
                Allocator::GetAllocator()->DeallocateAligned(passArray);
                a_self->unk0D8.head = nullptr;
                a_self->unk0D8.unk08 = 0x0;
            }
        }

        inline SafetyHookInline orig_BSLightingShaderProperty_ClearRenderPassArrays;

        inline void BSLightingShaderProperty_ClearRenderPassArrays(RE::BSLightingShaderProperty* a_self)
        {
            CleanAllocatedArrays(a_self);
            orig_BSLightingShaderProperty_ClearRenderPassArrays.call(a_self);
        }

        inline SafetyHookInline orig_BSLightingShaderProperty_dtor;

        inline void BSLightingShaderProperty_Dtor(RE::BSLightingShaderProperty* a_self)
        {
            CleanAllocatedArrays(a_self);
            orig_BSLightingShaderProperty_dtor.call(a_self);
        }

#ifdef SKYRIM_AE
        inline SafetyHookInline orig_BSLightingShaderProperty_deleting_dtor;

        inline void BSLightingShaderProperty_Deleting_Dtor(RE::BSLightingShaderProperty* a_self, byte* a_flags)
        {
            CleanAllocatedArrays(a_self);
            orig_BSLightingShaderProperty_deleting_dtor.call(a_self, a_flags);
        }
#endif

        inline void Install()
        {
            const REL::Relocation _14F0920{ RELOCATION_ID(100818, 107602) };
            orig_BSShadowDirectionalLight_14F0920 = safetyhook::create_inline(_14F0920.address(), BSShadowDirectionalLight_14F0920);

            REL::Relocation GetRenderPasses_ShadowMapOrMask{ RELOCATION_ID(99872, 106517), VAR_NUM(0x291, 0x295) };
            auto&           trampoline = SKSE::GetTrampoline();
            Patch           p(GetRenderPasses_ShadowMapOrMask.address(), SKSE::stl::unrestricted_cast<std::uintptr_t>(BSLightingShaderProperty_GetRenderPasses_ShadowMapOrMask_Detour));
            p.ready();
            GetRenderPasses_ShadowMapOrMask.write_branch<5>(trampoline.allocate(p));

            const REL::Relocation ClearArrays{ RELOCATION_ID(99881, 106526) };
            orig_BSLightingShaderProperty_ClearRenderPassArrays = safetyhook::create_inline(ClearArrays.address(), BSLightingShaderProperty_ClearRenderPassArrays);

            const REL::Relocation dtor{ RELOCATION_ID(99855, 106500) };
            orig_BSLightingShaderProperty_dtor = safetyhook::create_inline(dtor.address(), BSLightingShaderProperty_Dtor);

#ifdef SKYRIM_AE
            const REL::Relocation deleting_dtor{ REL::ID(106534) };
            orig_BSLightingShaderProperty_deleting_dtor = safetyhook::create_inline(deleting_dtor.address(), BSLightingShaderProperty_Deleting_Dtor);
#endif
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed bslightingshaderproperty shadowmap fix"sv);
    }
}
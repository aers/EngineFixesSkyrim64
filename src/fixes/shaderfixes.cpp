#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/Trampoline.h"

#include "patches.h"

#include "offsets.h"

namespace fixes
{
    struct BSRenderPass
    {
        const static int MaxLightInArrayC = 16;

        void* m_Shader;
        void* m_Property;
        void* m_Geometry;
        uint32_t m_TechniqueID;
        uint8_t Byte1C;
        uint8_t Byte1D;  // Instance index (offset) in an instance group?
        struct           // LOD information
        {
            uint8_t Index : 7;  // Also referred to as "texture degrade level"
            bool SingleLevel : 1;
        } m_Lod;
        uint8_t m_LightCount;
        uint16_t Word20;
        BSRenderPass* m_Previous;  // Previous sub-pass
        BSRenderPass* m_Next;      // Next sub-pass
        void** m_SceneLights;      // Pointer to an array of 16 lights (MaxLightInArrayC, restricted to 3?)
        uint32_t UnkDword40;       // Set from TLS variable. Pool index in BSRenderPassCache? Almost always zero.
    };

    typedef void (*_BSBatchRenderer_SetupAndDrawPass)(BSRenderPass* pass, uint32_t technique, bool alphaTest, uint32_t renderFlags);
    REL::Offset<_BSBatchRenderer_SetupAndDrawPass*> BSBatchRenderer_SetupAndDrawPass_origLoc(BSBatchRenderer_SetupAndDrawPass_offset);
    _BSBatchRenderer_SetupAndDrawPass BSBatchRenderer_SetupAndDrawPass_Orig;

    REL::Offset<std::uintptr_t> BSLightingShader_vtbl(BSLightingShader_vtbl_offset);

    uint32_t RAW_FLAG_RIM_LIGHTING = 1 << 11;
    uint32_t RAW_FLAG_DO_ALPHA_TEST = 1 << 20;
    uint32_t RAW_TECHNIQUE_EYE = 16;
    uint32_t RAW_TECHNIQUE_MULTILAYERPARALLAX = 11;
    uint32_t RAW_TECHNIQUE_ENVMAP = 1;
    uint32_t RAW_TECHNIQUE_PARALLAX = 3;

    void hk_BSBatchRenderer_SetupAndDrawPass(BSRenderPass* pass, uint32_t technique, bool alphaTest, uint32_t renderFlags)
    {
        if (*(uintptr_t*)pass->m_Shader == BSLightingShader_vtbl.GetAddress() && alphaTest)
        {
            auto rawTechnique = technique - 0x4800002D;
            auto subIndex = (rawTechnique >> 24) & 0x3F;
            if (subIndex != RAW_TECHNIQUE_EYE && subIndex != RAW_TECHNIQUE_ENVMAP && subIndex != RAW_TECHNIQUE_MULTILAYERPARALLAX && subIndex != RAW_TECHNIQUE_PARALLAX)
            {
                technique = technique | RAW_FLAG_DO_ALPHA_TEST;
                pass->m_TechniqueID = technique;
            }
        }

        BSBatchRenderer_SetupAndDrawPass_Orig(pass, technique, alphaTest, renderFlags);
    }

    bool PatchBSLightingShaderForceAlphaTest()
    {
        _VMESSAGE("- BSLightingShader Force Alpha Testing -");
        {
            struct BSBatchRenderer_SetupAndDrawPass_Code : SKSE::CodeGenerator
            {
                BSBatchRenderer_SetupAndDrawPass_Code() : SKSE::CodeGenerator()
                {
                    // 131F810
                    mov(ptr[rsp + 0x10], rbx);
                    mov(ptr[rsp + 0x18], rbp);
                    // 131F81A

                    // exit
                    jmp(ptr[rip]);
                    dq(BSBatchRenderer_SetupAndDrawPass_origLoc.GetAddress() + 0xA);
                }
            };

            BSBatchRenderer_SetupAndDrawPass_Code code;
            code.finalize();
            BSBatchRenderer_SetupAndDrawPass_Orig = _BSBatchRenderer_SetupAndDrawPass(code.getCode());

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(BSBatchRenderer_SetupAndDrawPass_origLoc.GetAddress(), unrestricted_cast<std::uintptr_t>(hk_BSBatchRenderer_SetupAndDrawPass));
        }
        _VMESSAGE("patched");
        return true;
    }

    REL::Offset<uintptr_t> BSLightingShader_SetupGeometry_ParallaxFixHookLoc(offset_BSLightingShader_SetupGeometry_ParallaxTechniqueFix, 0x577);

    bool PatchBSLightingShaderSetupGeometryParallax()
    {
        _VMESSAGE("- BSLightingShader SetupGeometry - Parallax Bug -");
        {
            struct BSLightingShader_SetupGeometry_Parallax_Code : SKSE::CodeGenerator
            {
                BSLightingShader_SetupGeometry_Parallax_Code() : SKSE::CodeGenerator()
                {
                    // orig code
                    and_(eax, 0x21C00);
                    cmovnz(edx, r8d);

                    // new code
                    cmp(ebx, 0x3);    // technique ID = PARALLAX
                    cmovz(edx, r8d);  // set eye update true

                    // jmp out
                    jmp(ptr[rip]);
                    dq(BSLightingShader_SetupGeometry_ParallaxFixHookLoc.GetAddress() + 0x9);
                };
            };

            BSLightingShader_SetupGeometry_Parallax_Code code;
            code.finalize();

            auto trampoline = SKSE::GetTrampoline();
            trampoline->Write6Branch(BSLightingShader_SetupGeometry_ParallaxFixHookLoc.GetAddress(), uintptr_t(code.getCode()));
        }
        _VMESSAGE("patched");
        return true;
    }
}

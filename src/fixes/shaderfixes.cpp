#include "skse64_common/Utilities.h"

#include "patches.h"

#include "offsets.h"

namespace fixes
{
    struct BSRenderPass
    {
        const static int MaxLightInArrayC = 16;

        void *m_Shader;
        void *m_Property;
        void *m_Geometry;
        uint32_t m_TechniqueID;
        uint8_t Byte1C;
        uint8_t Byte1D;				// Instance index (offset) in an instance group?
        struct						// LOD information
        {
            uint8_t Index : 7;		// Also referred to as "texture degrade level"
            bool SingleLevel : 1;
        } m_Lod;
        uint8_t m_LightCount;
        uint16_t Word20;
        BSRenderPass *m_Previous;	// Previous sub-pass
        BSRenderPass *m_Next;		// Next sub-pass
        void **m_SceneLights;	// Pointer to an array of 16 lights (MaxLightInArrayC, restricted to 3?)
        uint32_t UnkDword40;		// Set from TLS variable. Pool index in BSRenderPassCache? Almost always zero.
    };

    typedef void(*_BSBatchRenderer_SetupAndDrawPass)(BSRenderPass * pass, uint32_t technique, bool alphaTest, uint32_t renderFlags);
    RelocPtr<_BSBatchRenderer_SetupAndDrawPass> BSBatchRenderer_SetupAndDrawPass_origLoc(BSBatchRenderer_SetupAndDrawPass_offset);
    _BSBatchRenderer_SetupAndDrawPass BSBatchRenderer_SetupAndDrawPass_Orig;

    RelocAddr<uintptr_t> BSLightingShader_vtbl(BSLightingShader_vtbl_offset);

	uint32_t RAW_FLAG_RIM_LIGHTING = 1 << 11;
    uint32_t RAW_FLAG_DO_ALPHA_TEST = 1 << 20;
    uint32_t RAW_TECHNIQUE_EYE = 16;
	uint32_t RAW_TECHNIQUE_ENVMAP = 1;

    void hk_BSBatchRenderer_SetupAndDrawPass(BSRenderPass * pass, uint32_t technique, bool alphaTest, uint32_t renderFlags)
    {
        if (*(uintptr_t *)pass->m_Shader == BSLightingShader_vtbl.GetUIntPtr() && alphaTest)
        {
			auto rawTechnique = technique - 0x4800002D;
			auto subIndex = (rawTechnique >> 24) & 0x3F;
            if (subIndex != RAW_TECHNIQUE_EYE && subIndex != RAW_TECHNIQUE_ENVMAP)
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
            struct BSBatchRenderer_SetupAndDrawPass_Code : Xbyak::CodeGenerator
            {
                BSBatchRenderer_SetupAndDrawPass_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
                {
                    // 131F810
                    mov(ptr[rsp + 0x10], rbx);
                    mov(ptr[rsp + 0x18], rbp);
                    // 131F81A

                    // exit 
                    jmp(ptr[rip]);
                    dq(BSBatchRenderer_SetupAndDrawPass_origLoc.GetUIntPtr() + 0xA);
                }
            };

            void *codeBuf = g_localTrampoline.StartAlloc();
            BSBatchRenderer_SetupAndDrawPass_Code code(codeBuf);
            g_localTrampoline.EndAlloc(code.getCurr());

            BSBatchRenderer_SetupAndDrawPass_Orig = _BSBatchRenderer_SetupAndDrawPass(code.getCode());
            g_branchTrampoline.Write6Branch(BSBatchRenderer_SetupAndDrawPass_origLoc.GetUIntPtr(), GetFnAddr(hk_BSBatchRenderer_SetupAndDrawPass));
        }
        _VMESSAGE("patched");
        return true;
    }
}

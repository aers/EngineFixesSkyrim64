#include "../../skse64_common/Relocation.h"
#include "../../skse64_common/BranchTrampoline.h"

#include "../xbyak/xbyak.h"

// .text:0000000141309C60                 movss   xmm2, dword ptr[rbx + 0ACh]
// .text:0000000141309C68                 movss   xmm1, dword ptr[rbx + 0A8h]
// .text:0000000141309C70                 movss   xmm0, dword ptr[rbx + 0A4h]
// .text:0000000141309C78                 mov     eax, [rbx + 0A0h]
// .text:0000000141309C7E                 movss   dword ptr[rcx + rdx * 4 + 8], xmm1
// .text:0000000141309C84                 movss   dword ptr[rcx + rdx * 4 + 0Ch], xmm2
// .text:0000000141309C8A                 jmp     loc_141309D2D
//
// .text:0000000141309D2D                 movss   dword ptr[rcx + rdx * 4 + 4], xmm0
// .text:0000000141309D33                 mov[rcx + rdx * 4], eax
// .text:0000000141309D36                 mov     rax, cs:qword_14304F1F8; jumptable 0000000141309870 default case


namespace SnowSparkle
{
	RelocAddr<uintptr_t> BSLightingShaderMaterialSnow_vtbl(0x0187A1C0);
	RelocAddr<uintptr_t> BSLightingShader_SetupMaterial_Snow_Hook(0x013098D0);
	RelocAddr<uintptr_t> BSLightingShader_SetupMaterial_Snow_Exit(0x013099A6);

	bool Patch()
	{
		_MESSAGE("- snow sparkle -");
		_MESSAGE("patching BSLightingShader::SetupMaterial snow material case");
		{
			struct SetupMaterial_Snow_Hook_Code : Xbyak::CodeGenerator
			{
				SetupMaterial_Snow_Hook_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label vtblAddr;
					Xbyak::Label snowRetnLabel;
					Xbyak::Label exitRetnLabel;

					mov(rax, ptr[rip + vtblAddr]);
					cmp(rax, qword[rbx]);
					je("IS_SNOW");

					// not snow, fill with 1.0f
					mov(eax, 0x00000000); // 1.0f
					mov(dword[rcx + rdx * 4 + 0xC], eax);
					mov(dword[rcx + rdx * 4 + 0x8], eax);
					mov(dword[rcx + rdx * 4 + 0x4], eax);
					mov(dword[rcx + rdx * 4], eax);
					jmp(ptr[rip + exitRetnLabel]);

					// is snow, jump out to original except our overwritten instruction
					L("IS_SNOW");
					movss(xmm2, dword[rbx + 0xAC]);
					jmp(ptr[rip + snowRetnLabel]);

					L(vtblAddr);
					dq(BSLightingShaderMaterialSnow_vtbl.GetUIntPtr());

					L(snowRetnLabel);
					dq(BSLightingShader_SetupMaterial_Snow_Hook.GetUIntPtr() + 0x8);

					L(exitRetnLabel);
					dq(BSLightingShader_SetupMaterial_Snow_Exit.GetUIntPtr());
				}
			};

			void *codeBuf = g_localTrampoline.StartAlloc();
			SetupMaterial_Snow_Hook_Code code(codeBuf);
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(BSLightingShader_SetupMaterial_Snow_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
		}
		_MESSAGE("success");
		return true;
	}
}
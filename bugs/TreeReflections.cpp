#include "../skse64_common/SafeWrite.h"
#include "../skse64_common/BranchTrampoline.h"
#include "../skse64_common/Relocation.h"

#include "../xbyak/xbyak.h"

// .text:0000000141318C57                 jz      short loc_141318C5D
RelocAddr<uintptr_t> BSDistantTreeShader_VFunc3_Hook(0x01318C57);

bool PatchTreeReflections()
{
	{
		_MESSAGE("blah");
		struct PatchTreeReflection_Code : Xbyak::CodeGenerator
		{
			PatchTreeReflection_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;
				
				// current: if(bUseEarlyZ) v3 |= 0x10000u;
				// goal: if(bUseEarlyZ || v3 == 0) v3 |= 0x10000u;
				// if (bUseEarlyZ)
				// .text:0000000141318C50                 cmp     cs:bUseEarlyZ, r13b
				// need 6 bytes to branch jmp so enter here
				// enter 1318C57
				// .text:0000000141318C57                 jz      short loc_141318C5D
				jnz("CONDITION_MET");
				// edi = v3
				// if (v3 == 0)
				test(edi, edi);
				jnz("JMP_OUT");
				// .text:0000000141318C59                 bts     edi, 10h
				L("CONDITION_MET");
				bts(edi, 0x10);
				L("JMP_OUT");
				// exit 1318C5D
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(BSDistantTreeShader_VFunc3_Hook.GetUIntPtr() + 0x6);
			}
		};

		void *codeBuf = g_localTrampoline.StartAlloc();
		PatchTreeReflection_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		g_branchTrampoline.Write6Branch(BSDistantTreeShader_VFunc3_Hook.GetUIntPtr(), uintptr_t(code.getCode()));
	}
	_MESSAGE("blah2");
	return true;
}

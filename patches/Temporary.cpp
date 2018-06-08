#include "../../skse64_common/SafeWrite.h"
#include "../../skse64_common/BranchTrampoline.h"
#include "../../skse64/GameInput.h"
#include "../../xbyak/xbyak.h"

namespace Temporary
{
	void TempFixSKEE()
	{
		const auto skee = reinterpret_cast<uintptr_t>(GetModuleHandleA("skee64"));
		if (skee)
		{
			_MESSAGE("skee found, enabling temporary patch to fix XPMSSE 4.3 issue");
			if (*(uint32_t *)(skee + 0x6C10) != 0x0FD28548)
			{
				_MESSAGE("unknown skee version, canceling");
			}
			else
			{
				SafeWrite8(skee + 0x6C10, 0xC3);
				_MESSAGE("patched");
			}
		}
	}

	BranchTrampoline skseTrampoline;
	uintptr_t skse;

	void TempFixSKSECustomMenu()
	{
		if (skse)
		{
			// check version 2.0.7
			if (*(uint32_t *)(skse + 0x351D) == 0x012043C7)
			{
				_MESSAGE("patching custom menu mouse pointer");
				{
					struct CustomMenuCode : Xbyak::CodeGenerator
					{
						CustomMenuCode(void * buf, uintptr_t skse) : Xbyak::CodeGenerator(4096, buf)
						{
							Xbyak::Label IEDSingletonFuncPtr;

							// all asm here is from fixing the bug in skse and grabbing the compiled output
							mov(dword[rbx + 0x20], 1);
							call(ptr[rip+IEDSingletonFuncPtr]);
							test(rax, rax);
							jz("JMP_OUT");
							mov(rcx, ptr[rax + 0x70]);
							test(rcx, rcx);
							jz("OR_FLAG");
							mov(rax, ptr[rcx]);
							call(qword[rax + 0x38]);
							test(al, al);
							jnz("JMP_OUT");

							L("OR_FLAG");
							or(dword[rbx + 0x1C], 0x404);
							
							L("JMP_OUT");
							jmp(ptr[rip]);
							dq(skse + 0x3524);

							L(IEDSingletonFuncPtr);
							dq(GetFnAddr(InputEventDispatcher::GetSingleton));
						}
					};

					void * codeBuf = g_localTrampoline.StartAlloc();

					CustomMenuCode code(codeBuf, skse);
					g_localTrampoline.EndAlloc(code.getCurr());

					_MESSAGE("write branch");
					skseTrampoline.Write6Branch(skse + 0x351D, uintptr_t(code.getCode()));
				}
			}
		}
	}

	bool Patch()
	{
		_MESSAGE("- temporary fixes -");
		TempFixSKEE();
		skse = reinterpret_cast<uintptr_t>(GetModuleHandleA("skse64_1_5_39"));
		if (!skseTrampoline.Create(1024 * 64, (void*)skse))
		{
			_MESSAGE("couldn't create trampoline for skse dll. skipping skse patches");
			return true;
		}
		TempFixSKSECustomMenu();
		return true;
	}
}
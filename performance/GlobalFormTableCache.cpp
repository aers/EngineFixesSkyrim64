#include <tbb/concurrent_hash_map.h>
#include "../skse64_common/BranchTrampoline.h"
#include "../skse64/GameForms.h"
#include "../xbyak/xbyak.h"

#include "GlobalFormTableCache.h"
#include "DistantTreeCache.h"

RelocPtr<BSReadWriteLock> g_formTableLock(off_GLOBALFORMTABLELOCK);
RelocPtr<BSTHashMap<UInt32, TESForm *> *> g_formTable(off_GLOBALFORMTABLEPTR);

tbb::concurrent_hash_map<uint32_t, TESForm *> g_formMap[TES_FORM_MASTER_COUNT];

RelocAddr<_MutexLockRead> BSReadWriteLock_LockRead(off_BSREADWRITELOCK_LOCKREAD);
RelocAddr<_MutexUnlockRead> BSReadWriteLock_UnlockRead(off_BSREADWRITELOCK_UNLOCKREAD);

void UpdateFormCache(uint32_t FormId, TESForm *Value, bool Invalidate)
{
	const unsigned char masterId = (FormId & 0xFF000000) >> 24;
	const unsigned int baseId = (FormId & 0x00FFFFFF);

	if (Invalidate)
		g_formMap[masterId].erase(baseId);
	else
		g_formMap[masterId].insert(std::make_pair(baseId, Value));

	InvalidateCachedForm(FormId);
}

TESForm *GetFormById_Hook(unsigned int FormId)
{
	TESForm *formPointer = nullptr;

	const unsigned char masterId = (FormId & 0xFF000000) >> 24;	
	const unsigned int baseId = (FormId & 0x00FFFFFF);

	{
		tbb::concurrent_hash_map<uint32_t, TESForm *>::accessor accessor;

		if (g_formMap[masterId].find(accessor, baseId))
		{
			formPointer = accessor->second;
			return formPointer;
		}
	}

	// Try to use Bethesda's scatter table which is considerably slower
	BSReadWriteLock_LockRead(g_formTableLock);

	if (*g_formTable)
	{
		auto iter = (*g_formTable)->find(FormId);
		formPointer = (iter != (*g_formTable)->end()) ? iter->value : nullptr;
	}

	BSReadWriteLock_UnlockRead(g_formTableLock);

	UpdateFormCache(FormId, formPointer, false);

	return formPointer;
}

RelocAddr <UnknownFormFunction3_> origFunc3HookAddr(off_UNKNOWNFORMFUNCTION3);
UnknownFormFunction3_ origFunc3;
 __int64 UnknownFormFunction3(__int64 a1, __int64 a2, int a3, __int64 a4)
 {
 	UpdateFormCache(*(uint32_t *)a4, nullptr, true);

 	return origFunc3(a1, a2, a3, a4);
 }

 RelocAddr <UnknownFormFunction2_> origFunc2HookAddr(off_UNKNOWNFORMFUNCTION2);
 UnknownFormFunction2_ origFunc2;
 __int64 UnknownFormFunction2(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 **a5)
 {
 	UpdateFormCache(*formId, nullptr, true);

 	return origFunc2(a1, a2, a3, formId, a5);
 }

RelocAddr <UnknownFormFunction1_> origFunc1HookAddr(off_UNKNOWNFORMFUNCTION1);
UnknownFormFunction1_ origFunc1;
__int64 UnknownFormFunction1(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 *a5)
{
	UpdateFormCache(*formId, nullptr, true);

	return origFunc1(a1, a2, a3, formId, a5);
}

RelocAddr<UnknownFormFunction0_> origFunc0HookAddr(off_UNKNOWNFORMFUNCTION0);
UnknownFormFunction0_ origFunc0;
void UnknownFormFunction0(__int64 form, bool a2)
{
	UpdateFormCache(*(uint32_t *)(form + 0x14), nullptr, true);

	origFunc0(form, a2);
}

bool PatchGlobalFormTableCache()
{
	gLog.SetSource("FCache");

	_MESSAGE("Detouring GetFormById");
	g_branchTrampoline.Write6Branch(LookupFormByID.GetUIntPtr(), uintptr_t(GetFormById_Hook));
	_MESSAGE("Done");

	// TODO: write a generic detour instead
	_MESSAGE("Detouring global form table write functions");
	{
		struct UnknownFormFunction0_Code : Xbyak::CodeGenerator
		{
			UnknownFormFunction0_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				// 194970
				mov(r11, rsp);
				push(rbp);
				push(rsi);
				push(rdi);
				push(r12);
				// 194978

				// exit 
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(origFunc0HookAddr.GetUIntPtr() + 0x8);
			}
		};

		void *codeBuf = g_localTrampoline.StartAlloc();
		UnknownFormFunction0_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		origFunc0 = UnknownFormFunction0_(code.getCode());
		g_branchTrampoline.Write6Branch(origFunc0HookAddr.GetUIntPtr(), GetFnAddr(UnknownFormFunction0));
	}

	{
		struct UnknownFormFunction1_Code : Xbyak::CodeGenerator
		{
			UnknownFormFunction1_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				// 196070
				push(rdi);
				push(r14);
				push(r15);
				sub(rsp, 0x20);
				// 19607A

				// exit 
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(origFunc1HookAddr.GetUIntPtr() + 0xA);
			}
		};

		void *codeBuf = g_localTrampoline.StartAlloc();
		UnknownFormFunction1_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		origFunc1 = UnknownFormFunction1_(code.getCode());
		g_branchTrampoline.Write6Branch(origFunc1HookAddr.GetUIntPtr(), GetFnAddr(UnknownFormFunction1));
	}

	{
		struct UnknownFormFunction2_Code : Xbyak::CodeGenerator
		{
			UnknownFormFunction2_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				// 195DA0
				mov(ptr[rsp + 0x10], rbx);
				push(rsi);
				sub(rsp, 0x20);
				// 195DAA

				// exit 
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(origFunc2HookAddr.GetUIntPtr() + 0xA);
			}
		};

		void *codeBuf = g_localTrampoline.StartAlloc();
		UnknownFormFunction2_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		origFunc2 = UnknownFormFunction2_(code.getCode());
		g_branchTrampoline.Write6Branch(origFunc2HookAddr.GetUIntPtr(), GetFnAddr(UnknownFormFunction2));
	}

	{
		struct UnknownFormFunction3_Code : Xbyak::CodeGenerator
		{
			UnknownFormFunction3_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				Xbyak::Label retnLabel;

				// 196960
				push(rbp);
				push(rsi);
				push(r14);
				sub(rsp, 0x20);
				// 196969

				// exit 
				jmp(ptr[rip + retnLabel]);

				L(retnLabel);
				dq(origFunc3HookAddr.GetUIntPtr() + 0x9);
			}
		};

		void *codeBuf = g_localTrampoline.StartAlloc();
		UnknownFormFunction3_Code code(codeBuf);
		g_localTrampoline.EndAlloc(code.getCurr());

		origFunc3 = UnknownFormFunction3_(code.getCode());
		g_branchTrampoline.Write6Branch(origFunc3HookAddr.GetUIntPtr(), GetFnAddr(UnknownFormFunction3));
	}

	_MESSAGE("Done");
	
	return true;
}
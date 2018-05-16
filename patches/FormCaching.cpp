#include "../../skse64_common/BranchTrampoline.h"
#include "../../skse64_common/Relocation.h"
#include "../../skse64/GameTypes.h"
#include "../../skse64/GameData.h"
#include "../../skse64/GameRTTI.h"
#include "../../skse64/GameSettings.h"
#include "../../skse64/NiNodes.h"

#include "../../xbyak/xbyak.h"

#include "tbb/concurrent_hash_map.h"

#include "../tes/BSTHashMap.h"
#include "FormCaching.h"

namespace FormCaching
{
	// form cache
	// these can be found easily from the getformbyid function which has a pointer in skse,
	// but here's a signature anyway
	// E8 ? ? ? ? 45 8D 4F 04 -> GetFormByID
	RelocPtr<BSReadWriteLock> TESGlobalFormTableLock(0x01EEB150);
	RelocPtr<BSTHashMap<UInt32, TESForm *> *> TESGlobalFormTable(0x01EEACB8);

	// using originals since skse64 BSReadWriteLock implementation is broken
	// read lock/unlock are called from GetFormByID
	typedef void(*_MutexLockRead)(BSReadWriteLock * lock);
	RelocAddr<_MutexLockRead> BSReadWriteLock_LockRead(0x00C077E0);
	typedef void(*_MutexUnlockRead)(BSReadWriteLock * lock);
	RelocAddr<_MutexUnlockRead> BSReadWriteLock_UnlockRead(0x00C07AA0);

	// E8 ? ? ? ? 90 83 05 ? ? ? ? ? ->
	typedef void(*UnknownFormFunction0_)(__int64 form, bool a2);
	RelocAddr<UnknownFormFunction0_> origFunc0HookAddr(0x00194970);
	UnknownFormFunction0_ origFunc0;

	// E8 ? ? ? ? 84 C0 75 36 48 8B CD ->
	typedef __int64(*UnknownFormFunction1_)(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 *a5);
	RelocAddr <UnknownFormFunction1_> origFunc1HookAddr(0x00196070);
	UnknownFormFunction1_ origFunc1;

	// E8 ? ? ? ? 40 B5 01 49 8B CD ->
	typedef __int64(*UnknownFormFunction2_)(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 **a5);
	RelocAddr <UnknownFormFunction2_> origFunc2HookAddr(0x00195DA0);
	UnknownFormFunction2_ origFunc2;

	// E8 ? ? ? ? 48 8D 84 24 ? ? ? ? 48 89 44 24 ? 48 8D 44 24 ? 48 89 44 24 ? 4C 8D 4F 14 -> +0xC4 ->
	typedef __int64(*UnknownFormFunction3_)(__int64 a1, __int64 a2, int a3, __int64 a4);
	RelocAddr <UnknownFormFunction3_> origFunc3HookAddr(0x00196960);
	UnknownFormFunction3_ origFunc3;

	// distant tree alpha lod fade
	// E8 ? ? ? ? EB 0F 48 8B 43 18 ->
	typedef void(*UpdateLODAlphaFade_)(ResourceData * data);
	RelocAddr<UpdateLODAlphaFade_> UpdateLODAlphaFade_orig(0x004A8440);

	// E8 ? ? ? ? 66 89 47 04 ->
	typedef uint16_t(*Float2Half_)(float f);
	RelocAddr<Float2Half_> Float2Half(0x00D42750);

	// our maps
	tbb::concurrent_hash_map<uint32_t, TESForm *> globalFormCacheMap[TES_FORM_MASTER_COUNT];
	tbb::concurrent_hash_map<uint32_t, TESObjectREFR *> referencesFormCache;

	// no meaningful speedup
	// #define BITMAP_TEST

#ifdef BITMAP_TEST
	alignas(64) LONG CacheBitmap[0xFFFFFF / 32] = {};
#endif

	void InvalidateCachedForm(uint32_t formId)
	{
		const uint32_t maskedFormId = formId & 0x00FFFFFF;
#ifdef BITMAP_TEST
		_interlockedbittestandreset(&CacheBitmap[maskedFormId / 32], maskedFormId % 32);
#endif
		referencesFormCache.erase(maskedFormId);
	}

	void UpdateFormCache(uint32_t FormId, TESForm *Value, bool Invalidate)
	{
		const unsigned char masterId = (FormId & 0xFF000000) >> 24;
		const unsigned int baseId = (FormId & 0x00FFFFFF);

		if (Invalidate)
			globalFormCacheMap[masterId].erase(baseId);
		else
			globalFormCacheMap[masterId].insert(std::make_pair(baseId, Value));

		InvalidateCachedForm(FormId);
	}

	TESForm *GetFormById_Hook(unsigned int FormId)
	{
		TESForm *formPointer = nullptr;

		const unsigned char masterId = (FormId & 0xFF000000) >> 24;
		const unsigned int baseId = (FormId & 0x00FFFFFF);

		{
			tbb::concurrent_hash_map<uint32_t, TESForm *>::accessor accessor;

			if (globalFormCacheMap[masterId].find(accessor, baseId))
			{
				formPointer = accessor->second;
				return formPointer;
			}
		}

		// Try to use Bethesda's scatter table which is considerably slower
		BSReadWriteLock_LockRead(TESGlobalFormTableLock);

		if (*TESGlobalFormTable)
		{
			auto iter = (*TESGlobalFormTable)->find(FormId);
			formPointer = (iter != (*TESGlobalFormTable)->end()) ? iter->value : nullptr;
		}

		BSReadWriteLock_UnlockRead(TESGlobalFormTableLock);

		if (formPointer)
			UpdateFormCache(FormId, formPointer, false);

		return formPointer;
	}



	void UpdateLODAlphaFade_Hook(ResourceData *data)
	{
		//_MESSAGE("enter doalphafade");
		for (uint32_t i = 0; i < data->m_LODGroups.QSize(); i++)
		{
			LODGroup *group = data->m_LODGroups[i];

			for (uint32_t j = 0; j < group->m_LODInstances.QSize(); j++)
			{
				LODGroupInstance *instance = &group->m_LODInstances[j];
				const uint32_t maskedFormId = instance->FormId & 0x00FFFFFF;

				TESObjectREFR *refrObject = nullptr;
#ifdef BITMAP_TEST
				bool found = false;

				if (_bittest(&CacheBitmap[maskedFormId / 32], maskedFormId % 32) != 0)
				{
					found = true;
				}

				// Check if this instance was cached, otherwise search each plugin
				if (!found)
				{
					tbb::concurrent_hash_map<uint32_t, TESObjectREFR *>::accessor accessor;

					if (referencesFormCache.find(accessor, maskedFormId))
					{
						refrObject = accessor->second;			
						found = true;
					}
				}
				if (!found)
#endif
#ifndef BITMAP_TEST
				tbb::concurrent_hash_map<uint32_t, TESObjectREFR *>::accessor accessor;

				if (referencesFormCache.find(accessor, maskedFormId))
				{
					refrObject = accessor->second;
				}
				else
#endif
				{
					// Find first valid tree object by ESP/ESM load order
					for (int k = 0; k < DataHandler::GetSingleton()->modList.loadedMods.count; k++)
					{
						TESForm *form = GetFormById_Hook((k << 24) | maskedFormId);

						if (form && form->formType == kFormType_Reference)
							refrObject = DYNAMIC_CAST(form, TESForm, TESObjectREFR);

						if (refrObject)
						{
							TESForm * baseForm = refrObject->baseForm;

							if (baseForm)
							{
								// Checks if the form type is TREE (TESObjectTREE) and some other flag 0x40
								if ((baseForm->flags >> 6) & 1 || baseForm->formType == kFormType_Tree)
									break;
							}
						}

						refrObject = nullptr;
					}
#ifdef BITMAP_TEST
					if (!refrObject)
					{
						_interlockedbittestandset(&CacheBitmap[maskedFormId / 32], maskedFormId % 32);
					}

					//_MESSAGE("Cached 0x%08x", maskedFormId);
					
					else
#endif
						// Insert even if it's a null pointer
						referencesFormCache.insert(std::make_pair(maskedFormId, refrObject));
				}

				bool fullyHidden = false;
				float alpha = 1.0f;

				if (refrObject)
				{
					NiNode *node = refrObject->GetNiNode();
					// GetParentCell
					TESObjectCELL * cell = *(TESObjectCELL **)((__int64)refrObject + 0x60);

					// NiNode::GetAppCulled, TESObjectCELL::IsAttached
					if (node && !(*(BYTE *)((__int64)node + 0xF4) & 1) && *(uint8_t *)((__int64)cell + 0x44) == 7)
					{

						if (GetINISetting("bEnableStippleFade:Display")->data.u8)
						{

							void * fadeNode = node->Unk_05(); // BSFadeNode::GetAsFadeNode

							if (fadeNode)
							{
								alpha = 1.0f - *(float *)((__int64)fadeNode + 0x130);// BSFadeNode::fCurrentFade

								if (alpha <= 0.0f)
									fullyHidden = true;
							}

						}
						else
						{
							// No alpha fade - LOD trees will instantly appear or disappear
							fullyHidden = true;
						}
					}

					if (refrObject->flags & 0x820)
						fullyHidden = true;
				}

				const uint16_t halfFloat = Float2Half(alpha);
				
				if (instance->Alpha != halfFloat)
				{
					instance->Alpha = halfFloat;
					group->m_UnkByte24 = false;
				}

				if (instance->Hidden != fullyHidden)
				{
					instance->Hidden = fullyHidden;
					group->m_UnkByte24 = false;
				}

				if (fullyHidden)
					data->m_UnkByte82 = false;
			}
		}
	}


	__int64 UnknownFormFunction3(__int64 a1, __int64 a2, int a3, __int64 a4)
	{
		UpdateFormCache(*(uint32_t *)a4, nullptr, true);

		return origFunc3(a1, a2, a3, a4);
	}

	__int64 UnknownFormFunction2(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 **a5)
	{
		UpdateFormCache(*formId, nullptr, true);

		return origFunc2(a1, a2, a3, formId, a5);
	}

	__int64 UnknownFormFunction1(__int64 a1, __int64 a2, int a3, DWORD *formId, __int64 *a5)
	{
		UpdateFormCache(*formId, nullptr, true);

		return origFunc1(a1, a2, a3, formId, a5);
	}

	void UnknownFormFunction0(__int64 form, bool a2)
	{
		UpdateFormCache(*(uint32_t *)(form + 0x14), nullptr, true);

		origFunc0(form, a2);
	}

	bool Patch()
	{
		_MESSAGE("- form caching -");

		if (*(uint32_t *)LookupFormByID.GetUIntPtr() != 0x83485740)
		{
			_MESSAGE("sse fixes is installed and enabled. aborting form cache patch.");
			return false;
		}

		_MESSAGE("detouring GetFormById");
		g_branchTrampoline.Write6Branch(LookupFormByID.GetUIntPtr(), GetFnAddr(GetFormById_Hook));
		_MESSAGE("done");

		// TODO: write a generic detour instead
		_MESSAGE("detouring global form table write functions");
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
		_MESSAGE("done");
		
		_MESSAGE("detouring UpdateLODAlphaFade");
		g_branchTrampoline.Write6Branch(UpdateLODAlphaFade_orig.GetUIntPtr(), GetFnAddr(UpdateLODAlphaFade_Hook));
		_MESSAGE("done");

		_MESSAGE("success");

		return true;
	}
	
}
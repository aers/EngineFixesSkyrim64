#include <tbb/concurrent_hash_map.h>

#include "../skse64/GameData.h"
#include "../skse64/GameRTTI.h"
#include "../skse64/GameSettings.h"
#include "../skse64/NiNodes.h"

#include "../skse64_common/BranchTrampoline.h"

#include "GlobalFormTableCache.h"
#include "DistantTreeCache.h"

RelocAddr<UpdateLODAlphaFade_> UpdateLODAlphaFade_orig(off_BGSDISTANTTREEFADE_UPDATELODALPHAFADE);

// using this adds an incredibly minor speedup. i went from 164-165 fps to 166 fps in a riften test spot. will probably remove in cleanup.
//alignas(64) LONG CacheBitmap[0xFFFFFF / 32] = {};

tbb::concurrent_hash_map<uint32_t, TESObjectREFR *> InstanceFormCache;

void InvalidateCachedForm(uint32_t formId)
{
	const uint32_t maskedFormId = formId & 0x00FFFFFF;
	//_interlockedbittestandreset(&CacheBitmap[maskedFormId / 32], maskedFormId % 32);
	InstanceFormCache.erase(maskedFormId);
}

RelocAddr<Float2Half_> Float2Half(off_FLOAT2HALF);

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

			bool found = false;
			/*
			if (_bittest(&CacheBitmap[maskedFormId / 32], maskedFormId % 32) != 0)
			{
				found = true;
			}
			*/
			// Check if this instance was cached, otherwise search each plugin
			if (!found)
			{
				tbb::concurrent_hash_map<uint32_t, TESObjectREFR *>::accessor accessor;

				if (InstanceFormCache.find(accessor, maskedFormId))
				{
					refrObject = accessor->second;
					found = true;
					//_MESSAGE("cache lookup 0x%08x", maskedFormId);
				}
			}
			if (!found)
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
				/*
				if (!refrObject)
				{
					_interlockedbittestandset(&CacheBitmap[maskedFormId / 32], maskedFormId % 32);
				}
				//_MESSAGE("Cached 0x%08x", maskedFormId);
				// Insert even if it's a null pointer
				else*/
					InstanceFormCache.insert(std::make_pair(maskedFormId, refrObject));
			}
			
			bool fullyHidden = false;
			float alpha = 1.0f;

			
			if (refrObject)
			{
				NiNode *node = refrObject->GetNiNode();
				TESObjectCELL * cell = *(TESObjectCELL **)((__int64)refrObject + 0x60);

				if (node && !(*(BYTE *)((__int64)node + 0xF4) & 1) && *(uint8_t *)((__int64)cell + 0x44) == 7)
				{
					
					if (GetINISetting("bEnableStippleFade:Display")->data.u8)
					{
						
						void * fadeNode = node->Unk_05(); // GetAsFadeNode

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

			uint16_t halfFloat = Float2Half(alpha);

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

bool PatchDistantTreeCache()
{
	gLog.SetSource("DistTree");

	_MESSAGE("Detouring UpdateLODAlphaFade");
	g_branchTrampoline.Write6Branch(UpdateLODAlphaFade_orig.GetUIntPtr(), uintptr_t(UpdateLODAlphaFade_Hook));
	_MESSAGE("Done");

	return true;
}

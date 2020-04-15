#include "RE/Skyrim.h"
#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/Trampoline.h"

#include "tbb/concurrent_hash_map.h"

#include "patches.h"

namespace patches
{
    typedef void (*UpdateBlockVisibility_)(RE::BGSDistantTreeBlock* data);
    REL::Offset<UpdateBlockVisibility_> UpdateBlockVisibility_orig(UpdateBlockVisibility_orig_offset);

    typedef uint16_t (*Float2Half_)(float f);
    REL::Offset<Float2Half_> Float2Half(Float2Half_offset);

    typedef RE::TESForm* (*_LookupFormByID)(uint32_t id);
    REL::Offset<_LookupFormByID> LookupFormByID(LookupFormByID_offset);

    tbb::concurrent_hash_map<uint32_t, RE::TESObjectREFR*> referencesFormCache;

    void InvalidateCachedForm(uint32_t FormId)
    {
        referencesFormCache.erase(FormId & 0x00FFFFFF);
    }

    void hk_UpdateBlockVisibility(RE::BGSDistantTreeBlock* data)
    {
        for (auto& group : data->treeGroups)
        {
            for (auto& instance : group->instances)
            {
                const uint32_t maskedFormId = instance.id & 0x00FFFFFF;

                RE::TESObjectREFR* refrObject = nullptr;

                decltype(referencesFormCache)::accessor accessor;

                if (referencesFormCache.find(accessor, maskedFormId))
                {
                    refrObject = accessor->second;
                }
                else
                {
                    // Find first valid tree object by ESP/ESM load order
                    auto dataHandler = RE::TESDataHandler::GetSingleton();
                    for (uint32_t i = 0; i < dataHandler->compiledFileCollection.files.size(); i++)
                    {
                        RE::TESForm* form = LookupFormByID((i << 24) | maskedFormId);
                        if (form)
                            refrObject = form->AsReference();
                        if (refrObject)
                        {
                            auto baseObj = refrObject->GetBaseObject();
                            if (baseObj)
                            {
                                using STATFlags = RE::TESObjectSTAT::RecordFlags;
                                // Checks if the form type is TREE (TESObjectTREE) or if it has the kHasTreeLOD flag (TESObjectSTAT)
                                if (baseObj->formFlags & STATFlags::kHasTreeLOD || baseObj->Is(RE::FormType::Tree))
                                    break;
                            }
                        }

                        refrObject = nullptr;
                    }

                    // Insert even if it's a null pointer
                    referencesFormCache.insert(std::make_pair(maskedFormId, refrObject));
                }

                bool fullyHidden = false;
                float alpha = 1.0f;

                if (refrObject)
                {
                    auto obj3D = refrObject->Get3D();
                    auto cell = refrObject->GetParentCell();
                    if (obj3D && !obj3D->GetAppCulled() && cell->IsAttached())
                    {
                        static auto bEnableStippleFade = RE::GetINISetting("bEnableStippleFade:Display");  // ini settings are kept in a linked list, so we'll cache it
                        if (bEnableStippleFade->GetBool())
                        {
                            const auto fadeNode = obj3D->AsFadeNode();
                            if (fadeNode)
                            {
                                alpha = 1.0f - fadeNode->currentFade;
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

                    if (refrObject->IsInitiallyDisabled() || refrObject->IsDeleted())
                        fullyHidden = true;
                }

                const uint16_t halfFloat = Float2Half(alpha);

                if (instance.alpha != halfFloat)
                {
                    instance.alpha = halfFloat;
                    group->shaderPropertyUpToDate = false;
                }

                if (instance.hidden != fullyHidden)
                {
                    instance.hidden = fullyHidden;
                    group->shaderPropertyUpToDate = false;
                }

                if (fullyHidden)
                    data->allVisible = false;
            }
        }
    }

    bool PatchTreeLODReferenceCaching()
    {
        _VMESSAGE("- Tree LOD Reference Caching -");

        _VMESSAGE("detouring UpdateLODAlphaFade");
        auto trampoline = SKSE::GetTrampoline();
        trampoline->Write6Branch(UpdateBlockVisibility_orig.GetAddress(), unrestricted_cast<std::uintptr_t>(hk_UpdateBlockVisibility));
        _VMESSAGE("success");

        return true;
    }
}

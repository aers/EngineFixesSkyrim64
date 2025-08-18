#include "tree_lod_reference_caching.h"

#include "form_caching.h"

namespace Patches::TreeLodReferenceCaching
{
    namespace detail
    {
        inline bool HasTreeLod(const RE::TESBoundObject* a_boundObject)
        {
            using STATFlags = RE::TESObjectSTAT::RecordFlags;
            return a_boundObject->GetFormFlags() & STATFlags::kHasTreeLOD || a_boundObject->Is(RE::FormType::Tree);
        }

        void UpdateBlockVisibility(RE::BGSDistantTreeBlock* data)
        {
            for (auto& group : data->treeGroups)
            {
                for (auto& instance : group->instances)
                {
                    const std::uint32_t baseId = instance.id & 0x00FFFFFF;

                    RE::TESObjectREFR* objectReference = nullptr;

                    // new flag in later AE versions that indicates to not scan all files, hidden is no longer a bool
                    if ((static_cast<std::uint8_t>(instance.hidden) & 2) != 0)
                    {
                        if (RE::TESForm* form = FormCaching::detail::TESDataHandler_GetForm(instance.id))
                            objectReference = form->AsReference();
                        if (objectReference)
                        {
                            if (const auto baseObj = objectReference->GetBaseObject())
                            {
                                // if this isn't a tree then fail
                                if (!HasTreeLod(baseObj))
                                    objectReference = nullptr;
                            }
                        }
                    }
                    // otherwise try cache then scan all files
                    else
                    {
                        {
                            HashMap::const_accessor a;

                            if (g_treeReferenceCache.find(a, baseId)) {
                                objectReference = a->second;
                            }
                        }

                        if (!objectReference) {
                            // Find first valid tree object by ESP/ESM load order
                            const auto dataHandler = RE::TESDataHandler::GetSingleton();
                            for (std::uint32_t i = 0; i < dataHandler->compiledFileCollection.files.size(); i++)
                            {
                                if (RE::TESForm* form = FormCaching::detail::TESDataHandler_GetForm(i << 24 | baseId))
                                    objectReference = form->AsReference();
                                if (objectReference)
                                {
                                    if (const auto baseObj = objectReference->GetBaseObject())
                                    {
                                        // if object is a tree we found a valid reference
                                        if (HasTreeLod(baseObj))
                                            break;
                                    }
                                }
                                // continue searching if it wasn't a tree
                                objectReference = nullptr;
                            }

                            // Insert even if it's a null pointer
                            g_treeReferenceCache.emplace(baseId, objectReference);
                        }
                    }

                    // update visibility
                    bool fullyHidden = false;
                    float alpha = 1.0f;

                    if (objectReference)
                    {
                        const auto object3D = objectReference->Get3D();
                        const auto parentCell = objectReference->GetParentCell();
                        if (object3D && !object3D->GetAppCulled() && parentCell->IsAttached())
                        {
                            static auto bEnableStippleFade = RE::GetINISetting("bEnableStippleFade:Display");  // ini settings are kept in a linked list, so we'll cache it
                            if (bEnableStippleFade->GetBool())
                            {
                                const auto objectFadeNode = object3D->AsFadeNode();
                                if (objectFadeNode)
                                {
                                    alpha = 1.0f - objectFadeNode->currentFade;
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

                        if (objectReference->IsInitiallyDisabled() || objectReference->IsDeleted())
                            fullyHidden = true;
                    }

                    const std::uint16_t halfFloat = Float2Half(alpha);

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
    }
}
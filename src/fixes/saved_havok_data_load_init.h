#pragma once

namespace Fixes::SavedHavokDataLoadInit
{
    namespace detail
    {
        inline REL::Relocation<void(RE::NiAVObject*, RE::NiUpdateData*)> orig_NiAVObject_Update;

        inline void FlagForPrevWorldResetDownwards(RE::NiAVObject* a_self)
        {
            if (RE::NiNode* node = a_self->AsNode()) {
                for (auto& child : node->children) {
                    RE::NiAVObject* childPtr = child.get();
                    if (childPtr != nullptr)
                        FlagForPrevWorldResetDownwards(childPtr);
                }
            }
            a_self->flags02 |= 0x40;
        }

        inline void NiAVObject_Update(RE::NiAVObject* a_self, RE::NiUpdateData* a_data)
        {
            FlagForPrevWorldResetDownwards(a_self);
            orig_NiAVObject_Update(a_self, a_data);
        }

        inline void Install()
        {
            REL::Relocation target { RELOCATION_ID(19133, 19535), VAR_NUM(0x33F, 0x348) }; // TESObjectREFR::LoadHavokData post-load update
            orig_NiAVObject_Update = target.write_call<5>(NiAVObject_Update);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed saved havok data load init fix"sv);
    }
}
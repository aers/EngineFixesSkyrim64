#pragma once

namespace Patches::DisableSnowFlag
{
    namespace detail
    {
        inline REL::Relocation<bool(RE::TESLandTexture*, RE::TESFile*)> orig_TESLandTexture_Load;

        inline bool TESLandTexture_Load(RE::TESLandTexture* a_self, RE::TESFile* a_file)
        {
            const bool retVal = orig_TESLandTexture_Load(a_self, a_file);
            logger::info("LTEX {:X} {}", a_self->formID, a_self->shaderTextureIndex);
            if (retVal)
                a_self->shaderTextureIndex = 0;
            return retVal;
        }

        inline REL::Relocation<bool(RE::BGSMaterialObject*, RE::TESFile*)> orig_BGSMaterialObject_Load;

        inline bool BGSMaterialObject_Load(RE::BGSMaterialObject* a_self, RE::TESFile* a_file)
        {
            const bool retVal = orig_BGSMaterialObject_Load(a_self, a_file);
            logger::info("mato {:X} {}", a_self->formID, a_self->directionalData.flags.underlying());
            if (retVal)
                a_self->directionalData.flags.set(false, RE::BSMaterialObject::DIRECTIONAL_DATA::Flag::kSnow);
            return retVal;
        }

        inline void Install()
        {
            REL::Relocation LTEX { RE::TESLandTexture::VTABLE[0] };
            REL::Relocation MATO { RE::BGSMaterialObject::VTABLE[0] };

            orig_TESLandTexture_Load = LTEX.write_vfunc(6, TESLandTexture_Load);
            orig_BGSMaterialObject_Load = MATO.write_vfunc(6, BGSMaterialObject_Load);

        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed disable snow flag patches"sv);
    }
}
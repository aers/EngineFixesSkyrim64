#pragma once

namespace TextureLoadCrash
{
    namespace detail
    {
        class UnkClass
        {
        public:
            std::byte pad[0x88];
        };

        class Texture
        {
        public:
            std::byte _pad[0x20];
            int unk;
            std::byte _pad2[0x4];
        };

        inline REL::Relocation<void(RE::BSResourceNiBinaryStream* a_self, /* RE::BSTSmartPointer<RE::BSResource::Stream> */ RE::BSResource::Stream** a_stream, bool a_fullReadHint, bool a_useOwnBuffer)> _ctor { RELOCATION_ID(69637, 71015) };
        inline REL::Relocation<void(RE::BSResourceNiBinaryStream* a_self)> _dtor { RELOCATION_ID(69638 , 71016)};
        inline REL::Relocation<REX::W32::HRESULT(REX::W32::ID3D11Device*, RE::BSResourceNiBinaryStream*, Texture**, UnkClass*, std::uint32_t, std::uint32_t)> _dxLoadTexture { RELOCATION_ID(75721, 77533) };

        inline void BSShaderResourceManager_LoadTexture(void*, RE::NiSourceTexture* a_texture)
        {
            // cant use CLib's ctor
            std::byte streamBytes[sizeof(RE::BSResourceNiBinaryStream)];
            auto* stream = reinterpret_cast<RE::BSResourceNiBinaryStream*>(&streamBytes);
            _ctor(stream, &a_texture->unk40, 1, 0);
            Texture* texture = nullptr;
            UnkClass unkClass{};
            REX::W32::HRESULT result = _dxLoadTexture(RE::BSGraphics::Renderer::GetDevice(), stream, &texture, &unkClass, 0, 0);
            if (result != 0) {
                logger::info("texture load failed - file path {} result {:X}"sv, a_texture->name.c_str(), result);
                a_texture->rendererTexture = nullptr;
            }
            else {
                texture->unk = 1;
                a_texture->rendererTexture = reinterpret_cast<RE::BSGraphics::Texture*>(texture);
            }
            _dtor(stream);
        }

        inline void Install()
        {
            REL::Relocation vtable { RE::VTABLE_BSShaderResourceManager[0] };
            vtable.write_vfunc(26, BSShaderResourceManager_LoadTexture);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed texture load crash fix"sv);
    }
}
#pragma once

namespace Fixes::TextureLoadCrash
{
    namespace detail
    {
        inline std::uint32_t TotalLoadFails = 0;

        inline REL::Relocation<void(RE::BSResourceNiBinaryStream* a_self, /* RE::BSTSmartPointer<RE::BSResource::Stream> */ RE::BSResource::Stream** a_stream, bool a_fullReadHint, bool a_useOwnBuffer)> BSResourceNiBinaryStream_ctorFromResourceStream{ RELOCATION_ID(69637, 71015) };
        inline REL::Relocation<void(RE::BSResourceNiBinaryStream* a_self)>                                                                                                                                BSResourceNiBinaryStream_dtor{ RELOCATION_ID(69638, 71016) };
        inline REL::Relocation<REX::W32::HRESULT(REX::W32::ID3D11Device*, RE::BSResourceNiBinaryStream*, RE::BSGraphics::Texture**, RE::BSGraphics::DDSInfo*, std::uint32_t, std::uint32_t)>              BSGraphics_Renderer_LoadTextureFromStream{ RELOCATION_ID(75721, 77533) };

        inline void BSShaderResourceManager_LoadTexture(void*, RE::NiSourceTexture* a_texture)
        {
            // CommmonLib's constructor is a different one, so we can't use it here
            std::byte streamBytes[sizeof(RE::BSResourceNiBinaryStream)];
            auto*     stream = reinterpret_cast<RE::BSResourceNiBinaryStream*>(&streamBytes);
            BSResourceNiBinaryStream_ctorFromResourceStream(stream, &a_texture->unk40, 1, 0);
            RE::BSGraphics::Texture* texture = nullptr;
            RE::BSGraphics::DDSInfo  ddsInfo{};
            REX::W32::HRESULT        result = BSGraphics_Renderer_LoadTextureFromStream(RE::BSGraphics::Renderer::GetDevice(), stream, &texture, &ddsInfo, 0, 0);
            if (FAILED(result)) {
                TotalLoadFails++;
                a_texture->rendererTexture = nullptr;
                switch (result) {
                case HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED):
                    logger::warn("texture load failed due to unsupported format - file path {}"sv, a_texture->name.c_str());
                    break;
                case E_OUTOFMEMORY:
                    logger::warn("texture load failed due to out of memory - file path {}"sv, a_texture->name.c_str());
                    break;
                case HRESULT_FROM_WIN32(ERROR_HANDLE_EOF):
                case HRESULT_FROM_WIN32(ERROR_INVALID_DATA):
                case E_FAIL:
                    logger::warn("texture load failed due to invalid DDS file - file path {}"sv, a_texture->name.c_str());
                    break;
                case E_POINTER:
                case E_INVALIDARG:  // shouldn't be possible unless the game is fundamentally broken
                    logger::warn("texture load failed unexpectedly - file path {}"sv, a_texture->name.c_str());
                    break;
                default:
                    logger::warn("texture load failed with unknown result code {:X} - file path {}"sv, static_cast<std::uint32_t>(result), a_texture->name.c_str());
                }
            } else {
                texture->unk20 = 1;
                a_texture->rendererTexture = texture;
            }
            BSResourceNiBinaryStream_dtor(stream);
        }

        inline void Install()
        {
            REL::Relocation vtable{ RE::VTABLE_BSShaderResourceManager[0] };
            vtable.write_vfunc(26, BSShaderResourceManager_LoadTexture);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed texture load crash fix"sv);
    }
}
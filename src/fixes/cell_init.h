#pragma once

namespace Fixes::CellInit
{
    namespace detail
    {
        struct ExtraDataList
        {
            static RE::BGSLocation* GetLocation(const RE::ExtraDataList* a_self)
            {
                const auto cell = SKSE::stl::adjust_pointer<RE::TESObjectCELL>(a_self, -0x48);
                auto loc = _GetLocation(a_self);
                if (!cell->IsInitialized())
                {
                    const auto file = cell->GetFile();
                    auto formID = static_cast<RE::FormID>(reinterpret_cast<std::uintptr_t>(loc));
                    RE::TESForm::AddCompileIndex(formID, file);
                    loc = RE::TESForm::LookupByID<RE::BGSLocation>(formID);
                }
                return loc;
            }

            static inline REL::Relocation<decltype(GetLocation)> _GetLocation;
        };

    }
    inline void Install()
    {
        REL::Relocation target { REL::ID(18474), 0x110 };
        detail::ExtraDataList::_GetLocation = target.write_call<5>(detail::ExtraDataList::GetLocation);

        logger::info("installed cell init fix"sv);
    }
}
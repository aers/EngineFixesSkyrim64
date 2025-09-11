#pragma once

namespace Patches::INISettingCollection
{
    namespace detail
    {
        inline bool INISettingCollection_Open(RE::INISettingCollection* a_self, bool)
        {
            // "subKey" == filename
            if (std::filesystem::exists(a_self->subKey)) {
                a_self->handle = a_self;
                return true;
            }

            return false;
        }

        inline void Install()
        {
            REL::Relocation vtable { RE::INISettingCollection::VTABLE[0] };
            vtable.write_vfunc(5, INISettingCollection_Open);
        }
    }

    inline void Install()
    {
        detail::Install();
        logger::info("installed INISettingCollection patch"sv);
    }
}
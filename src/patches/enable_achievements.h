#pragma once

namespace Patches::EnableAchievementsWithMods
{
    namespace detail
    {
        struct Patch final : Xbyak::CodeGenerator
        {
            Patch()
            {
                xor_(rax, rax);
                ret();
            }
        };
    }

    inline void Install()
    {
        REL::Relocation target { RELOCATION_ID(13647, 441528) };

        target.write_fill(REL::INT3, VAR_NUM(0x73, 0x6E));

        detail::Patch p;
        p.ready();
        target.write(std::span{ p.getCode<const std::byte*>(), p.getSize()});

        logger::info("installed enable achievements with mods patch");
    }
}
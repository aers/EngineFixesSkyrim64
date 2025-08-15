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
        REL::Relocation target { REL::ID(441528) };

        target.write_fill(REL::INT3, 0x6E);

        detail::Patch p;
        p.ready();
        target.write(std::span(p.getCode<const std::byte*>(), p.getSize()));

        REX::INFO("installed enable achievements with mods patch");
    }
}
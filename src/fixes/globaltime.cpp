namespace fixes
{
    bool PatchGlobalTime()
    {
        logger::trace("- global time fix -"sv);

        const REL::ID timer{ 523661 };
        constexpr std::array todo = {
            std::make_pair(50118, 0xA91),  // BookMenu
            std::make_pair(51614, 0x1BC),  // SleepWaitMenu
        };

        for (const auto& elem : todo)
        {
            const REL::Relocation<std::uintptr_t> target{ REL::ID(elem.first), elem.second };
            const auto offset = static_cast<std::int32_t>(timer.address() - (target.address() + 0x8));
            REL::safe_write(target.address() + 0x4, offset);
        }

        logger::trace("success"sv);
        return true;
    }
}

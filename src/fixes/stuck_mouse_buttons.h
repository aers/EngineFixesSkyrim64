#pragma once

namespace Fixes::StuckMouseButtons
{
    namespace detail
    {
        enum MouseButtonFlags : std::uint32_t
        {
            kLMB = 1u << 0,
            kRMB = 1u << 1,
            kMMB = 1u << 2,
        };

        class MenuOpenCloseEventSink : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
        {
        public:
            static MenuOpenCloseEventSink* GetSingleton()
            {
                static MenuOpenCloseEventSink singleton;
                return &singleton;
            }

        private:
            MenuOpenCloseEventSink() = default;
            MenuOpenCloseEventSink(const MenuOpenCloseEventSink&) = delete;
            MenuOpenCloseEventSink& operator=(const MenuOpenCloseEventSink&) = delete;

            static void TrySendMouseUp(RE::IMenu* a_menu, std::uint32_t a_buttonMask)
            {
                auto* cursor = RE::MenuCursor::GetSingleton();
                if (!cursor)
                    return;

                constexpr std::pair<std::uint32_t, std::uint32_t> kButtonTable[] = {
                    { kLMB, 0 },
                    { kRMB, 1 },
                    { kMMB, 2 },
                };

                const std::uint32_t buttonIndex = a_buttonMask - 1;

                RE::GFxMouseEvent mouseUp(
                    RE::GFxEvent::EventType::kMouseUp,
                    buttonIndex,
                    cursor->cursorPosX,
                    cursor->cursorPosY,
                    0.0f,
                    0);

                a_menu->uiMovie->HandleEvent(mouseUp);
            }

            RE::BSEventNotifyControl ProcessEvent(
                const RE::MenuOpenCloseEvent* a_event,
                RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
            {
                if (!a_event || !a_event->opening)
                    return RE::BSEventNotifyControl::kContinue;

                SKSE::GetTaskInterface()->AddTask([menuName = a_event->menuName]() {
                    auto* ui = RE::UI::GetSingleton();
                    if (!ui)
                        return;

                    RE::IMenu* openedMenu = nullptr;
                    auto       it = ui->menuMap.find(menuName.c_str());
                    if (it != ui->menuMap.end())
                        openedMenu = it->second.menu.get();

                    for (auto& menuPtr : ui->menuStack) {
                        auto* menu = menuPtr.get();

                        if (!menu || !menu->uiMovie || !menu->UsesCursor() || menu == openedMenu)
                            continue;

                        float         fx = 0.0f, fy = 0.0f;
                        std::uint32_t buttonMask = 0;
                        menu->uiMovie->GetMouseState(0, &fx, &fy, &buttonMask);

                        if (buttonMask == 0)
                            continue;

                        TrySendMouseUp(menu, buttonMask);
                    }
                });

                return RE::BSEventNotifyControl::kContinue;
            }
        };
    }

    inline void Install()
    {
        RE::UI::GetSingleton()->AddEventSink(detail::MenuOpenCloseEventSink::GetSingleton());
        logger::info("installed stuck mouse buttons fix"sv);
    }
}
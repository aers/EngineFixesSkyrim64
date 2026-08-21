#pragma once

namespace Fixes::StuckMouseButtons
{
    namespace detail
    {
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

            static void TrySendMouseUp(RE::IMenu* a_menu)
            {
                auto* cursor = RE::MenuCursor::GetSingleton();
                if (!cursor)
                    return;

                RE::GFxMouseEvent mouseUp(
                    RE::GFxEvent::EventType::kMouseUp,
                    0,
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

                SKSE::GetTaskInterface()->AddTask([menuName = std::string(a_event->menuName.c_str())]() {
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
                        std::uint32_t fButtons = 0;
                        menu->uiMovie->GetMouseState(0, &fx, &fy, &fButtons);

                        if (fButtons == 0)
                            continue;

                        TrySendMouseUp(menu);
                        return;
                    }
                });

                return RE::BSEventNotifyControl::kContinue;
            }
        };
    }

    inline void Install()
    {
        RE::UI::GetSingleton()->AddEventSink(detail::MenuOpenCloseEventSink::GetSingleton());
        REX::INFO("installed stuck mouse buttons fix"sv);
    }
}
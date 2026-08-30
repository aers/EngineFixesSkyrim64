#pragma once

namespace Fixes::StuckMouseButtons
{
    namespace detail
    {
        // button mask bit -> mouse index in Scaleform
        inline constexpr std::uint32_t kLMB = 1u << 0;
        inline constexpr std::uint32_t kRMB = 1u << 1;
        inline constexpr std::uint32_t kMMB = 1u << 2;

        inline constexpr std::array<std::pair<std::uint32_t, std::uint32_t>, 3> kButtonTable{ {
            { kLMB, 0 },
            { kRMB, 1 },
            { kMMB, 2 },
        } };

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

            static void SendMouseUp(RE::IMenu& a_menu, float a_x, float a_y, std::uint32_t a_buttonMask)
            {
                for (const auto& [mask, index] : kButtonTable) {
                    if (a_buttonMask & mask) {
                        const RE::GFxMouseEvent mouseUp(
                            RE::GFxEvent::EventType::kMouseUp,
                            index,
                            a_x,
                            a_y,
                            0.0f,
                            0);
                        a_menu.uiMovie->HandleEvent(mouseUp);
                    }
                }
            }

            RE::BSEventNotifyControl ProcessEvent(
                const RE::MenuOpenCloseEvent* a_event,
                RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
            {
                if (!a_event || !a_event->opening)
                    return RE::BSEventNotifyControl::kContinue;

                auto* ui = RE::UI::GetSingleton();
                if (!ui)
                    return RE::BSEventNotifyControl::kContinue;

                RE::IMenu* openedMenu = nullptr;
                if (auto it = ui->menuMap.find(a_event->menuName.c_str()); it != ui->menuMap.end())
                    openedMenu = it->second.menu.get();

                for (const auto& menuPtr : ui->menuStack) {
                    auto* menu = menuPtr.get();

                    if (!menu || !menu->uiMovie || !menu->UsesCursor() || menu == openedMenu)
                        continue;

                    float         fx = 0.0f, fy = 0.0f;
                    std::uint32_t buttonMask = 0;
                    menu->uiMovie->GetMouseState(0, &fx, &fy, &buttonMask);

                    if (buttonMask == 0)
                        continue;

                    SendMouseUp(*menu, fx, fy, buttonMask);
                }

                return RE::BSEventNotifyControl::kContinue;
            }
        };
    }

    inline void Install()
    {
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            REX::WARN("failed to install stuck mouse buttons fix: UI not available"sv);
            return;
        }

        ui->AddEventSink(detail::MenuOpenCloseEventSink::GetSingleton());
        REX::INFO("installed stuck mouse buttons fix"sv);
    }
}

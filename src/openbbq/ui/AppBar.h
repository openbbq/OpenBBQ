#pragma once

// #include <openbbq/display/widget/Button.h>
#include <display/ui/Background.h>
#include <display/ui/Label.h>
#include <openbbq/control/ControlSignal.h>

namespace bbq::ui
{
    using namespace display;
    using namespace display::ui;

    class AppBar : public Background
    {
    public:
        struct ViewModel
        {
            ControlSignal<float> &battery;
        };

        AppBar(StylePtr style, const ViewModel &vm) : Background(style), model(vm) {}

        static WindowPtr create(StylePtr style, const String &text, const ViewModel &vm)
        {
            auto nav = std::make_shared<AppBar>(style, vm);
            nav->_title = std::make_shared<Label>(style, text, DrawContext::LEFT);
            nav->_battery = Label::create(style, "", DrawContext::RIGHT);

            uint16_t batteryWidth = style->font()->measure("0.00v").width();
            nav->addRight(nav->_battery, batteryWidth);
            nav->addFill(nav->_title);
            return nav;
        }

        Size measureHandler(const Size &available) const override
        {
            return _title->measureHandler(available) + Size(0, 4);
        }

        void loopHandler() override
        {
            Background::loopHandler();

            _battery->text(String(model.battery.value(), 2)+"v");
        }

    private:
        ViewModel model;
        // std::shared_ptr<Button> _menu;
        std::shared_ptr<Label> _title;
        std::shared_ptr<Label> _battery;
    };
}

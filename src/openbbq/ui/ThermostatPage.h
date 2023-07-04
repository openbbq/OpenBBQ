#pragma once

#include "StyleSheet.h"

#include <display/ui/Background.h>
#include <display/ui/Label.h>
#include <display/ui/Solid.h>

#include <openbbq/control/ControlThermostat.h>
#include <openbbq/sys/SysFan.h>

namespace bbq::ui
{
    using namespace display;
    using namespace display::ui;

    class ThermostatBar : public Background
    {
    public:
        struct ViewModel
        {
            String caption;
            ControlThermostat &thermostat;
        };

        ThermostatBar(StylePtr style, const ViewModel &vm) : Background(style), model(vm) {}

        static WindowPtr create(const StyleSheet &style, const ViewModel &vm)
        {
            int16_t largeHeight = style.LargeMonospace->font()->measure("0").height();
            int16_t smallHeight = style.SmallText->font()->measure("0").height();

            auto p = std::make_shared<ThermostatBar>(style.Default, vm);
            p->current = p->addRight<Label>(style.LargeMonospace, " 000.0", DrawContext::RIGHT);
            p->name = p->addLeft<Label>(style.Default, p->model.caption, DrawContext::LEFT);
            if (largeHeight / 2 > smallHeight)
            {
                p->addTop<Solid>(style.Default, Size(0, largeHeight / 2 - smallHeight));
            }
            p->setpoint = p->addTop<Label>(style.SmallText, "000 F", DrawContext::RIGHT);
            p->delta = p->addTop<Label>(style.SmallText, "+00.0/m", DrawContext::RIGHT);
            return p;
        }

        ViewModel model;
        WindowPtr name;
        WindowPtr current;
        WindowPtr setpoint;
        WindowPtr delta;

        void loopHandler() override
        {
            Background::loopHandler();

            current->text(String(model.thermostat.smoothed.output.value(), 1));
            setpoint->text(String(model.thermostat.temperature.value(), 0) + " F");

            String deltaText = String(model.thermostat.rate.output.value(), 1) + "/m";
            if (deltaText[0] == '-')
            {
                delta->text(deltaText);
            }
            else
            {
                delta->text("+" + deltaText);
            }
        }

        Size measureHandler(const Size &available) const override
        {
            // return name->measureHandler(available) + current->measureHandler(available) + delta->measureHandler(available) + Size(0, 2);
            Size sz = current->measureHandler(available);
            Serial.printf("ThermostatBar::measureHandler current:{%d,%d}\n", sz.width(), sz.height());
            return sz;
            // Size sz1 = current->measureHandler(available);
        };
    };

    class FanBar : public Background
    {
    public:
        struct ViewModel
        {
            String caption;
            SysFan &fan;
        };

        FanBar(StylePtr style, const ViewModel &vm) : Background(style), model(vm) {}

        static WindowPtr create(const StyleSheet &style, const ViewModel &vm)
        {
            auto p = std::make_shared<FanBar>(style.Default, vm);
            p->current = p->addRight<Label>(style.SmallMonospace, "000.0 %", DrawContext::RIGHT);
            p->name = p->addLeft<Label>(style.SmallText, p->model.caption, DrawContext::LEFT);
            return p;
        }

        ViewModel model;
        WindowPtr name;
        WindowPtr current;

        void loopHandler() override
        {
            Background::loopHandler();
            current->text(String(model.fan.power.value(), 1) + " %");
        }

        Size measureHandler(const Size &available) const override
        {
            return current->measureHandler(available);
        };
    };

    class ThermostatPage : public Background
    {
    public:
        struct ViewModel
        {
            FanBar::ViewModel fan;
            ThermostatBar::ViewModel cook;
            ThermostatBar::ViewModel food1;
            ThermostatBar::ViewModel food2;
            ThermostatBar::ViewModel food3;
        };

        ThermostatPage(StylePtr style, const ViewModel &vm) : Background(style), model(vm) {}

        static WindowPtr create(const StyleSheet &style, const ViewModel &model)
        {
            auto p = std::make_shared<ThermostatPage>(style.Default, model);
            p->addTop(ThermostatBar::create(style, p->model.cook));
            p->addTop(FanBar::create(style, p->model.fan));
            p->addTop(ThermostatBar::create(style, p->model.food1));
            p->addTop(ThermostatBar::create(style, p->model.food2));
            p->addTop(ThermostatBar::create(style, p->model.food3));
            return p;
        }

        ViewModel model;
    };
}
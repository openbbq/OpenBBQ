#pragma once

#include "StyleSheet.h"
#include "Resources.h"

#include <display/ui/Background.h>
#include <display/ui/Label.h>
#include <display/ui/Solid.h>
#include <display/ui/IconButton.h>

#include <openbbq/control/ControlSignal.h>
#include <openbbq/control/ControlThermostat.h>
#include <openbbq/sys/SysFan.h>

namespace bbq::ui
{
    using namespace display;
    using namespace display::ui;

    class ThermostatListItem : public Background
    {
    public:
        struct ViewModel
        {
            String caption;
            ControlThermostat &thermostat;
        };

        ThermostatListItem(StylePtr style, const ViewModel &vm) : Background(style), model(vm) {}

        static WindowPtr create(const StyleSheet &style, const ViewModel &vm)
        {
            int16_t smallHeight = style.Default->font()->measure("0").height();
            int16_t largeHeight = style.Large->font()->measure("0").height();

            auto p = std::make_shared<ThermostatListItem>(style.Default, vm);
            p->current = p->addRight<Label>(style.Large, " 000.0", DrawContext::RIGHT);
            p->name = p->addLeft<Label>(style.Default, p->model.caption, DrawContext::LEFT);
            if (largeHeight / 2 > smallHeight)
            {
                p->addTop<Solid>(style.Default, Size(0, largeHeight / 2 - smallHeight));
            }
            p->setpoint = p->addTop<Label>(style.Default, "000 F", DrawContext::RIGHT);
            p->delta = p->addTop<Label>(style.Default, "+00.0/m", DrawContext::RIGHT);
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

            if (model.thermostat.faults.value() == 0)
            {
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
            else
            {
                // use delta to show fault string, which enables temp value to
                // continue to display data
                delta->text(model.thermostat.fault.value());
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

    class FanListItem : public Background
    {
    public:
        struct ViewModel
        {
            String caption;
            SysFan &fan;
            ControlSignal<String>& mode;
        };

        FanListItem(StylePtr style, const ViewModel &vm) : Background(style), model(vm) {}

        static WindowPtr create(const StyleSheet &style, const ViewModel &vm)
        {
            auto &res = getResources();

            auto p = std::make_shared<FanListItem>(style.Default, vm);
            p->enabled = p->addRight<IconButton>(style.Default, res.Icons, Resources::toggle_off, Resources::toggle_on_fill);
            p->current = p->addRight<Label>(style.Default, "000.0 %", DrawContext::RIGHT);
            p->name = p->addLeft<Label>(style.Default, p->model.caption, DrawContext::LEFT);
            return p;
        }

        ViewModel model;
        WindowPtr name;
        WindowPtr current;
        WindowPtr enabled;

        void loopHandler() override
        {
            Background::loopHandler();
            current->text(String(model.fan.signal.value(), 1) + " %");
            if (model.mode.value() == "off")
            {
                enabled->toggled(false);
            }
            else
            {
                enabled->toggled(true);
            }
        }

        bool clickHandler(const WindowPtr &ptr) override
        {
            if (ptr == enabled)
            {
                if (model.mode.value() == "off")
                {
                    model.mode.value("heat");
                }
                else
                {
                    model.mode.value("off");
                }
                return true;
            }
            return false;
        }

        Size measureHandler(const Size &available) const override
        {
            return enabled->measureHandler(available);
        };
    };

    class ThermostatList : public Background
    {
    public:
        struct ViewModel
        {
            FanListItem::ViewModel fan;
            ThermostatListItem::ViewModel cook;
            ThermostatListItem::ViewModel food1;
            ThermostatListItem::ViewModel food2;
            ThermostatListItem::ViewModel food3;
        };

        ThermostatList(StylePtr style, const ViewModel &vm) : Background(style), model(vm) {}

        static WindowPtr create(const StyleSheet &style, const ViewModel &model)
        {
            auto p = std::make_shared<ThermostatList>(style.Default, model);
            p->addTop(ThermostatListItem::create(style, p->model.cook));
            p->addTop(FanListItem::create(style, p->model.fan));
            p->addTop(ThermostatListItem::create(style, p->model.food1));
            p->addTop(ThermostatListItem::create(style, p->model.food2));
            p->addTop(ThermostatListItem::create(style, p->model.food3));
            return p;
        }

        ViewModel model;
    };
}
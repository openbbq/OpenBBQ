#pragma once

#include <display/ui/Screen.h>
#include <display/ui/Background.h>
#include "AppBar.h"
#include "NavigationBar.h"
#include "ThermostatList.h"
#include "TuneView.h"

namespace bbq::ui
{
    class App : public Screen
    {
    public:
        struct ViewModel
        {
            AppBar::ViewModel appBar;
            int active;
            ThermostatList::ViewModel thermostats;
            TuneView::ViewModel tune;
        };
        App(Interface *interface, StylePtr style, const ViewModel &vm) : Screen(interface, style), model(vm) {}

        void build(const StyleSheet &ss)
        {
            ss_ = ss;

            addTop(appBar = AppBar::create(ss.System, "Open BBQ", model.appBar));
            addBottom(navigationBar = NavigationBar::create(ss.System), 40);
            addFill(main = std::make_shared<ScrollWindow>(ss.Default));

            model.active = -1;
            activate(NavigationBar::GRILL);
        }

        bool clickHandler(const WindowPtr &clicked) override
        {
            return activate(clicked->id());
        }

        bool activate(int view)
        {
            if (model.active == view)
            {
                return true;
            }
            if (!navigationBar->activate(view))
            {
                return false;
            }
            model.active = view;
            switch (model.active)
            {
            case NavigationBar::CHART:
                std::make_shared<Background>(style())->parent(main);
                break;
            case NavigationBar::NOTIFICATIONS:
                break;
            case NavigationBar::GRILL:
                ThermostatList::create(ss_, model.thermostats)->parent(main);
                break;
            case NavigationBar::TUNE:
                TuneView::create(ss_.Default, model.tune)->parent(main);
                break;
            case NavigationBar::SETTINGS:
                break;
            default:
                return false;
            }
            return true;
        }

        StyleSheet ss_;
        ViewModel model;

        WindowPtr appBar;
        std::shared_ptr<NavigationBar> navigationBar;
        std::shared_ptr<ScrollWindow> main;
    };
} // namespace bbq::ui

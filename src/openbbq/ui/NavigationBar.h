#pragma once

#include <display/Window.h>
#include <display/ui/IconButton.h>
#include "Resources.h"

namespace bbq::ui
{
    using namespace display;
    using namespace display::ui;

    class NavigationBar : public ParentWindow
    {
    public:
        using ParentWindow::ParentWindow; // inherits constructors

        static std::shared_ptr<NavigationBar> create(StylePtr style)
        {
            auto &res = getResources();

            auto p = std::make_shared<NavigationBar>(style);
            p->Charts = p->add<IconButton>(style, res.Icons, res.show_chart, res.show_chart_fill);
            p->Notifications = p->add<IconButton>(style, res.Icons, res.notifications, res.notifications_fill);
            p->Grill = p->add<IconButton>(style, res.Icons, res.grill, res.grill_fill);
            p->Tune = p->add<IconButton>(style, res.Icons, res.tune, res.tune_fill);
            p->Settings = p->add<IconButton>(style, res.Icons, res.settings, res.settings_fill);

            p->Charts->id(CHART);
            p->Notifications->id(NOTIFICATIONS);
            p->Grill->id(GRILL);
            p->Tune->id(TUNE);
            p->Settings->id(SETTINGS);
            return p;
        }

        enum Items
        {
            UNSPECIFIED = 100,
            CHART = 101,
            NOTIFICATIONS,
            GRILL,
            TUNE,
            SETTINGS,
        };

        WindowPtr Charts;
        WindowPtr Notifications;
        WindowPtr Grill;
        WindowPtr Tune;
        WindowPtr Settings;

        bool activate(int view)
        {
            bool activated = false;
            for (auto it = children().begin(); it != children().end(); it++)
            {
                auto ptr = *it;
                if (ptr->id() == view)
                {
                    ptr->activated(true);
                    activated = true;
                }
                else
                {
                    ptr->activated(false);
                }
            }
            return activated;
        }

        Size measureHandler(const Size &available) const override
        {
            ParentWindow::measureHandler(available);

            int height = 0;
            for (auto it = children().begin(); it != children().end(); it++)
            {
                Rect rc = (*it)->measureHandler(available);
                if (height < rc.height())
                {
                    height = rc.height();
                }
            }
            return Size(available.width(), height);
        }

        void resizeHandler() override
        {
            ParentWindow::resizeHandler();

            Rect rc = content();
            int count = children().size();
            int index = 0;
            int left = rc.left();
            for (auto it = children().begin(); it != children().end(); it++, index++)
            {
                int right = rc.left() + rc.width() * (index + 1) / count;
                Rect pos(left, rc.top(), right, rc.bottom());
                log_d("page button positioned %s", pos.toString().c_str());
                (*it)->position(pos);
                left = right;
            }
        }

        bool clickHandler(const WindowPtr &clicked) override
        {
            for (auto it = children().begin(); it != children().end(); it++)
            {
                auto ptr = *it;
                if (ptr == clicked)
                {
                    ptr->activated(true);
                }
                else
                {
                    ptr->activated(false);
                }
            }

            ParentWindow::clickHandler(clicked);
            return true;
        }
    };

} // namespace ui

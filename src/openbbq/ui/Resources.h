#pragma once

#include <display/Font.h>

namespace bbq::ui
{
    struct Resources
    {
        display::FontPtr Default;
        display::FontPtr Large;
        display::FontPtr Icons;

        enum IconGlyphs
        {
            toggle_off = 'A',
            toggle_on_fill,
            show_chart,
            show_chart_fill,
            notifications,
            notifications_fill,
            grill,
            grill_fill,
            tune,
            tune_fill,
            settings,
            settings_fill,
        };
    };

    const Resources &getResources();
} // namespace bbq::ui

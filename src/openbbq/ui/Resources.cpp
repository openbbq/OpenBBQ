#include <Arduino.h>
#include "Resources.h"

#include <Font_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/TomThumb.h>

#include <Font_FTO.h>
#include <Outlines/Roboto-Light-9.h>
#include <Outlines/Roboto-Light-24.h>
#include <Outlines/MaterialIcons-Regular-18.h>
#include <Outlines/MaterialIcons-Outlined-18.h>

namespace bbq::ui
{
    using namespace display;

    FT_GlyphSlotRec_ icon_glyphs[] = {
        materialicons::outlined18::uE9F5,
        materialicons::regular18::uE9F6,
        materialicons::outlined18::uE6E1,
        materialicons::regular18::uE6E1,
        materialicons::outlined18::uE7F4,
        materialicons::regular18::uE7F4,
        materialicons::outlined18::uEA47,
        materialicons::regular18::uEA47,
        materialicons::outlined18::uE429,
        materialicons::regular18::uE429,
        materialicons::outlined18::uE8B8,
        materialicons::regular18::uE8B8,
    };
    GlyphRange icon_ranges[] = {
        {'A', 12},
        {0, 0},
    };

    GlyphFont icons = {icon_glyphs, icon_ranges,
                       {
                           36,     // x_ppem
                           36,     // y_ppem
                           294912, // x_scale
                           294912, // y_scale
                           2304,   // ascender (36px)
                           0,      // descender (0px)
                           2304,   // height (36px)
                           2304    // max_advance (36px)
                       }};

    Resources makeResources()
    {
        Resources resources;
        resources.Default = Font_FTO::create(&roboto::light9::font, 1);
        resources.Large = Font_FTO::create(&roboto::light24::font, 1);
        resources.Icons = Font_FTO::create(&icons, 1);
        return resources;
    }

    const Resources &getResources()
    {
        static Resources resources = makeResources();
        return resources;
    }
} // namespace bbq::ui

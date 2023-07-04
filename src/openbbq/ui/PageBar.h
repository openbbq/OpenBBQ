#pragma once

#include <display/Window.h>
#include <display/ui/Button.h>

#include <FreeTypePrinter.h>
#include <Outlines/MaterialIcons-Regular-18.h>
#include <Outlines/MaterialIcons-Outlined-18.h>

namespace bbq::ui
{
    using namespace display;
    using namespace display::ui;

    class IconButton : public Button
    {
    public:
        IconButton(StylePtr style, StylePtr pressedStyle, StylePtr toggledStyle, FT_GlyphSlot glyph, FT_GlyphSlot toggledGlyph)
            : Button(style, pressedStyle, toggledStyle), _glyph(glyph), _toggledGlyph(toggledGlyph)
        {
        }

        Size measureHandler(const Size &available) const override
        {
            // TODO(loudej) this is purely the box that contains marking
            return Size(
                _glyph->metrics.width >> 6,
                _glyph->metrics.height >> 6);
        }

        void drawHandler(DrawContext *ctx) override
        {
            auto s0 = style();
            auto s1 = toggledStyle();
            auto s2 = pressedStyle();

            Color fg = toggled() ? s1->foreground() : s0->foreground();
            Color bg = pressed() ? s2->background() : s0->background();

            FT_GlyphSlot glyph = toggled() ? _toggledGlyph : _glyph;
            Size sz(
                glyph->metrics.width >> 6,
                glyph->metrics.height >> 6);

            Rect rc = content();
            Point cursor = (rc.size() * 64 +
                            Size(-glyph->metrics.horiBearingX * 2, glyph->metrics.horiBearingY * 2) +
                            Size(-glyph->metrics.width, -glyph->metrics.height)) /
                           128;

            Rect rcGlyph = Rect(rc.origin() + (rc.size() - sz) / 2, sz);

            FreeTypePrinter printer;
            printer.callback(IconButton_drawCallback, ctx);
            printer.setForeground(fg);
            printer.setBackground(bg);
            // TODO(loudej) need an EM box to center? Also need to set clip box for efficiency.
            printer.setCursor(cursor.x(), cursor.y() - 1);

            ctx->draw(rc, bg);
            // ctx->draw(rcGlyph, 0xFD20);                       // debug
            // ctx->draw(Rect(rc.origin(), Size(2, 2)), 0x03E0); // debug
            if (toggled())
            {
                printer.print(_toggledGlyph);
            }
            else
            {
                printer.print(_glyph);
            }
            ctx->exclude(rc);
        }

    private:
        static void IconButton_drawCallback(int32_t x, int32_t y, uint32_t w, uint32_t color, void *param)
        {
            static_cast<DrawContext *>(param)->draw(Rect(x, y, x + w, y + 1), color);
        }

        FT_GlyphSlot _glyph = nullptr;
        FT_GlyphSlot _toggledGlyph = nullptr;
    };

    class PageBar : public ParentWindow
    {
    public:
        using ParentWindow::ParentWindow; // inherits constructors

        static WindowPtr create(StylePtr style, StylePtr pressedStyle, StylePtr toggledStyle)
        {
            auto p = std::make_shared<PageBar>(style);
            p->selGraphs = p->add<IconButton>(style, pressedStyle, toggledStyle, &materialicons::outlined18::uE6E1, &materialicons::regular18::uE6E1);
            p->selAlerts = p->add<IconButton>(style, pressedStyle, toggledStyle, &materialicons::outlined18::uE7F4, &materialicons::regular18::uE7F4);
            p->selProbes = p->add<IconButton>(style, pressedStyle, toggledStyle, &materialicons::outlined18::uEA47, &materialicons::regular18::uEA47);
            p->selAdjustments = p->add<IconButton>(style, pressedStyle, toggledStyle, &materialicons::outlined18::uE429, &materialicons::regular18::uE429);
            p->selSettings = p->add<IconButton>(style, pressedStyle, toggledStyle, &materialicons::outlined18::uE8B8, &materialicons::regular18::uE8B8);
            return p;
        }

        WindowPtr selGraphs;
        WindowPtr selAlerts;
        WindowPtr selProbes;
        WindowPtr selAdjustments;
        WindowPtr selSettings;

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
                    ptr->toggled(true);
                }
                else
                {
                    ptr->toggled(false);
                }
            }
            return true;
        }
    };

} // namespace ui

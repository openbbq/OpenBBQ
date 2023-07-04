#pragma once

//#include <openbbq/display/widget/Button.h>
#include <display/ui/Background.h>

namespace bbq::ui
{
    using namespace display;
    using namespace display::ui;

    class NavButton : public Button
    {
    public:
        using Button::Button; // inherits constructors

        Size measureHandler(const Size &available) const override
        {
            return Size(available.height() * 1.2, available.height());
        };

        void drawHandler(DrawContext *dc) override
        {
            Rect rc = content();
            StylePtr s = activeStyle();

            int barHeight = (rc.height() - 2) / 7;
            Rect barsBox(Size(barHeight * 6, barHeight * 5));
            barsBox = barsBox.offset((rc.size() - barsBox.size()) / 2);

            Rect bar(barsBox.left(), barsBox.top(), barsBox.right(), barsBox.top() + barHeight);
            for (int i = 0; i != 3; ++i)
            {
                dc->draw(bar, s->foreground());
                dc->exclude(bar);
                bar = bar.offset(Point(0, barHeight * 2));
            }

            // TODO - the exclude above has a bug - it's not letting background draw between or under bars
            dc->draw(rc, s->background());
            dc->exclude(rc);
        }
    };

    class NavTitle : public Label
    {
    public:
        using Label::Label; // inherits constructors
    };

    class NavBar : public Background
    {
    public:
        using Background::Background; // inherits constructors

        static WindowPtr create(StylePtr style, StylePtr pressedStyle, const String &text)
        {
            auto nav = std::make_shared<NavBar>(style);
            nav->_menu = std::make_shared<NavButton>(style, pressedStyle, "=");
            nav->_menu = std::make_shared<NavButton>(style, pressedStyle, "=");
            nav->_title = std::make_shared<NavTitle>(style, text, DrawContext::LEFT);

            nav->addLeft(nav->_menu, -1);
            nav->addFill(nav->_title);
            return nav;
        }

        Size measureHandler(const Size &available) const override
        {
            return _title->measureHandler(available) + Size(0, 4);
        };

    private:
        std::shared_ptr<Button> _menu;
        std::shared_ptr<Label> _title;
    };
}

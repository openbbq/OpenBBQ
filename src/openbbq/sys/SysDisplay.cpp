#include <Arduino.h>
#include "SysDisplay.h"

#include <Display.h>
#include <display/ui/Screen.h>
#include <display/ui/Background.h>
#include <display/ui/Label.h>
#include <display/ui/Button.h>

#include <OpenBBQ.h>
#include <openbbq/ui/NavBar.h>
#include <openbbq/ui/ThermostatPage.h>
#include <openbbq/ui/PageBar.h>

#include <Font_FTO.h>
#include <Outlines/Roboto-Light-9.h>
#include <Outlines/Roboto-Light-24.h>

#include <Font_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/TomThumb.h>

#include <Adafruit_ILI9341.h>

using namespace display;
using namespace display::ui;
using namespace bbq::ui;

SysDisplay::SysDisplay(Interface *interface)
    : _interface(interface)
{
}

bool SysDisplay::begin(const ThermostatPage::ViewModel &model)
{
    FontPtr defaultFont = Font_FTO::create(&roboto::light9::font, 1);

    FontPtr smallFont = defaultFont;

    FontPtr smallMonoFont = defaultFont;
    FontPtr largeMonoFont = Font_FTO::create(&roboto::light24::font, 1);

    StyleSheet ss;
    ss.Default = Style::create(ILI9341_WHITE, ILI9341_BLACK, defaultFont);
    ss.Title = Style::create(ILI9341_WHITE, ILI9341_BLACK, defaultFont);

    ss.SmallText = Style::create(ILI9341_WHITE, ILI9341_BLACK, smallFont);

    ss.SmallMonospace = Style::create(ILI9341_WHITE, ILI9341_BLACK, smallMonoFont);
    ss.LargeMonospace = Style::create(ILI9341_WHITE, ILI9341_BLACK, largeMonoFont);

    StylePtr untoggledButton = Style::create(ILI9341_WHITE, ILI9341_DARKGREY, defaultFont);
    StylePtr pressedButton = Style::create(ILI9341_LIGHTGREY, ILI9341_BLACK, defaultFont);
    StylePtr toggledButton = Style::create(ILI9341_WHITE, ILI9341_BLACK, defaultFont);

    auto screen = _interface->begin<Screen>(ss.Default);
    auto nav = NavBar::create(ss.Title, pressedButton, "Open BBQ");
    auto main = Background::create(Style::create(ILI9341_CYAN));
    auto select = PageBar::create(untoggledButton, pressedButton, toggledButton);
    screen->addTop(nav);
//    screen->addTop<Solid>(Style::create(ILI9341_LIGHTGREY), Size(2, 2));
    screen->addBottom(select, 40);
    screen->addFill(main);

    // select->addRight(Button::create(untoggledButton, pressedButton, toggledButton, "E"), 30);
    // select->addRight(Button::create(untoggledButton, pressedButton, toggledButton, "D"), 30);
    // select->addRight(Button::create(untoggledButton, pressedButton, toggledButton, "C"), 30);
    // select->addRight(Button::create(untoggledButton, pressedButton, toggledButton, "B"), 30);
    // select->addRight(Button::create(untoggledButton, pressedButton, toggledButton, "A"), 30);

    main->addFill(ThermostatPage::create(ss, model));

    // _interface->screen(screen);

    return true;
}

bool SysDisplay::loop()
{
    _interface->loop();

    return true;
}

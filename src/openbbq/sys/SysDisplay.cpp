#include <Arduino.h>
#include "SysDisplay.h"

#include <Display.h>
#include <display/ui/Screen.h>
#include <display/ui/Background.h>
#include <display/ui/Label.h>
#include <display/ui/Button.h>

#include <OpenBBQ.h>
#include <openbbq/ui/Resources.h>
#include <openbbq/ui/App.h>

#include <Adafruit_ILI9341.h>

using namespace display;
using namespace display::ui;
using namespace bbq::ui;

SysDisplay::SysDisplay(Interface *interface)
    : _interface(interface)
{
}

using material_color_utilities::Argb;
using material_color_utilities::ArgbFromRgb;
using material_color_utilities::MaterialDarkColorScheme;
using material_color_utilities::MaterialLightColorScheme;
using material_color_utilities::Scheme;

bool SysDisplay::begin(const App::ViewModel &model)
{
    auto &res = getResources();
    auto scheme = std::make_shared<Scheme>(MaterialLightColorScheme(0x378b00));

    StyleSheet ss;
    ss.Default = Style::create(scheme, SchemeColors::Application, res.Default);
    ss.Large = Style::create(scheme, SchemeColors::Application, res.Large);
    ss.System = Style::create(scheme, SchemeColors::System, res.Default);

    auto screen = _interface->begin<App>(ss.Default, model);
    screen->build(ss);
    return true;
}

bool SysDisplay::loop()
{
    _interface->loop();

    return true;
}

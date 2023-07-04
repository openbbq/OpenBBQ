#pragma once

#include <Display.h>

#include <openbbq/ui/ThermostatPage.h>
#include <openbbq/control/ControlSignal.h>

class SysDisplay
{
public:
    SysDisplay(display::Interface *interface);

    display::Interface *interface() const { return _interface; }

    bool begin(const bbq::ui::ThermostatPage::ViewModel &model);
    bool loop();

    WithConnect<ControlSignal<float>> line1 = {0};
    WithConnect<ControlSignal<float>> line2 = {0};
    WithConnect<ControlSignal<String>> line3 = {""};
    WithConnect<ControlSignal<String>> line4 = {""};

private:
    // display::ClippedAdafruitGFX<Adafruit_ILI9341> _tft;
    //  TFT_eSPI _tft;

    display::Interface *_interface;

    display::WindowPtr _floating;
    display::WindowPtr _line1;
    display::WindowPtr _line2;
    display::WindowPtr _line3;
    display::WindowPtr _line4;
};

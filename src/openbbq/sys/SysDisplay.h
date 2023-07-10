#pragma once

#include <Display.h>

#include <openbbq/ui/App.h>
#include <openbbq/control/ControlSignal.h>

class SysDisplay
{
public:
    SysDisplay(display::Interface *interface);

    display::Interface *interface() const { return _interface; }

    bool begin(const bbq::ui::App::ViewModel &model);
    bool loop();

private:
    display::Interface *_interface;
};

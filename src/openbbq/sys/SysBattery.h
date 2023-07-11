
#pragma once

#include <openbbq/control/ControlSignal.h>

class SysBattery
{
public:
    SysBattery(int8_t pin) : _pin(pin) {}

    bool begin(uint32_t periodMs = 5000);
    bool loop(uint32_t timeMs);

    void read();

    ControlSignal<float> output = {0};

private:
    int8_t _pin = -1;
    uint32_t _periodMs = 5000;
    uint32_t _lastMs = 0;
};

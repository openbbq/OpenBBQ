
#pragma once

#include <Arduino.h>
#include <Ticker.h>

#include <openbbq/control/ControlSignal.h>

class SysFan
{
public:
    SysFan(int8_t pin) : _pin(pin) {}

    bool begin(uint32_t periodMs, uint32_t backgroundMs = 0);
    bool loop(uint32_t time);

    void write(uint32_t time);

    WithRange<WithConnect<ControlSignal<float>>> signal = {0, 0, 100};
    WithRange<ControlSignal<float>> factor = {1, 0, 1};
    ControlSignal<float> output = {0};

private:
    static void callback(SysFan *self) { self->write(millis()); }

    int8_t _pin = -1;

    uint32_t _backgroundMs = 0;
    Ticker _background;

    uint32_t _periodMs = 1000;
};

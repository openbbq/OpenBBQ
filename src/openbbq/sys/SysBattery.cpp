#include <Arduino.h>
#include "SysBattery.h"

bool SysBattery::begin(uint32_t periodMs)
{
    _periodMs = periodMs;
    _lastMs = millis();
    return true;
}

bool SysBattery::loop(uint32_t timeMs)
{
    if (timeMs - _lastMs < _periodMs)
    {
        return false;
    }
    read();
    _lastMs = timeMs;
    return true;
}

void SysBattery::read()
{
    int adc = analogRead(_pin);

    float volts = adc * 2 * 3.3 / 4096;
    output = volts;
    log_i("battery %f volts", volts);
}

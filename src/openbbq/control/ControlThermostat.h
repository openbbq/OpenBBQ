#pragma once

#include <openbbq/control/ControlSignal.h>
#include <openbbq/control/ControlLowPassFilter.h>
#include "ControlRate.h"

#include <openbbq/config/Config.h>

class ControlThermostat : public ConfigObject
{
public:
    ControlThermostat()
    {
        ConfigObject::add("mode", &mode);
        ConfigObject::add("temp", &temperature);

        smoothed.alpha = 20;
        smoothed.signal.connect(current);
        rate.signal.connect(smoothed.output);
    }

    bool begin()
    {
        return smoothed.begin(1000) &&
               rate.begin(100);
    }
    bool loop(int32_t timeMs)
    {
        if (current.value() == 0 || faults.value() != 0)
        {
            return false;
        }
        return smoothed.loop(timeMs) &&
               rate.loop(timeMs);
    }

    WithConfig<ControlSignal<float>> temperature = {70};
    WithConfig<ControlSignal<String>> mode = {"off"};
    WithConnect<ControlSignal<float>> current = {0};
    WithConnect<ControlSignal<int>> faults = {0};
    WithConnect<ControlSignal<String>> fault = {""};
    ControlSignal<String> action = {"idle"};

    ControlLowPassFilter smoothed;
    ControlRate rate;

private:
};

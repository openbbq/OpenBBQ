#pragma once

#include <Arduino.h>

#include <openbbq/control/ControlSignal.h>

class ControlLowPassFilter
{
public:
    bool begin(int32_t periodMs);
    bool loop(int32_t timeMs);

    WithRange<WithConfig<ControlSignal<float>>> alpha = {5, 0, 100};
    WithConnect<ControlSignal<float>> signal = {0};
    ControlSignal<float> output = {0};

private:
    // loop
    int32_t _ms = 1000;

    // last loop
    bool _hasLast = false; // for initial loop, or reset
    int32_t _timeLast = 0;
    float _outputLast = 0;
};

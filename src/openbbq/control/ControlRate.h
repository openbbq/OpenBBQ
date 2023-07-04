#pragma once

#include "ControlSignal.h"
#include "ControlPeriod.h"
#include "ControlLowPassFilter.h"

class ControlRate
{
public:
    ControlRate()
    {
        _filter.signal.connect(delta);
        _filter.alpha.value(3);
        output.connect(_filter.output);
    }

    bool begin(int32_t periodMs);
    bool loop(int32_t timeMs);

    WithConnect<ControlSignal<float>> signal = {0};
    ControlSignal<float> delta = {0};
    WithConnect<ControlSignal<float>> output = {0};

private:
    ControlPeriod _period;
    ControlLowPassFilter _filter;

    // loop
    int32_t _ms = 1000;

    // last loop
    bool _hasLast = false; // for initial loop, or reset
    int32_t _timeLast = 0;
    float _signalLast = 0;
};

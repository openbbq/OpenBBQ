#include <Arduino.h>
#include "ControlRate.h"

bool ControlRate::begin(int32_t periodMs)
{
    return _period.begin(periodMs, true) &&
           _filter.begin(1000);
}

bool ControlRate::loop(int32_t timeMs)
{
    if (!_hasLast && signal.value() == 0)
    {
        // guard against spike
        return false;
    }

    if (_period.loop(timeMs))
    {
        if (_hasLast)
        {
            delta = (signal - _signalLast) / (timeMs - _timeLast) * 60000;

            log_v("rate %f = (%f - %f) / (%d - %d) * 60000",
                  delta.value(),
                  signal.value(),
                  _signalLast,
                  timeMs,
                  _timeLast);
        }
        else
        {
            delta = 0;
        }

        _hasLast = true;
        _timeLast = timeMs;
        _signalLast = signal;
    }

    if (_hasLast)
    {
        return _filter.loop(timeMs);
    }
    return false;
}

#include <Arduino.h>
#include <openbbq/control/ControlLowPassFilter.h>
#include <openbbq/DebugSerial.h>

bool ControlLowPassFilter::begin(int32_t ms)
{
    _hasLast = false;
    _ms = ms;
    return true;
}

bool ControlLowPassFilter::loop(int32_t time)
{
    if (!_hasLast)
    {
        output.value(signal);
    }
    else if (time <= _timeLast + 50)
    {
        return false;
    }
    else
    {
        float periods = float(time - _timeLast) / _ms;
        float beta = pow(1 - alpha / 100, periods);

        output = (1 - beta) * signal + beta * _outputLast;
    }

    _hasLast = true;
    _timeLast = time;
    _outputLast = output;

    return true;
}

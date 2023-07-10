#include "SysFan.h"
#include <openbbq/DebugSerial.h>

bool SysFan::begin(uint32_t periodMs, uint32_t backgroundMs)
{
    if (_pin == -1)
    {
        return false;
    }

    if (periodMs == 0)
    {
        return false;
    }

    _periodMs = periodMs;
    _backgroundMs = backgroundMs;

    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    return true;
}

bool SysFan::loop(uint32_t time)
{
    output = signal * factor;

    if (_backgroundMs == 0)
    {
        write(time);
    }
    else if (!_background.active())
    {
        log_i("Fan %dms background update rate", _backgroundMs);
        _background.attach_ms(_backgroundMs, callback, this);
    }
    return true;
}

void SysFan::write(uint32_t time)
{
    if (_pin == -1)
    {
        return;
    }

    uint32_t sample = time % _periodMs;
    uint32_t outputMs = uint32_t(_periodMs * output / 100);

    if (sample < outputMs)
    {
        digitalWrite(_pin, HIGH);
    }
    else
    {
        digitalWrite(_pin, LOW);
    }
}

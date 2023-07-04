#include "SysTemperature.h"

#include <utility>

bool SysTemperature::begin(int32_t backgroundMs)
{
    if (_readyPin != -1)
    {
        pinMode(_readyPin, INPUT);
    }
    if (!_max31856.begin())
    {
        return false;
    }

    _max31856.setThermocoupleType(MAX31856_TCTYPE_T);
    _max31856.setConversionMode(MAX31856_CONTINUOUS);

    if (backgroundMs != 0)
    {
        _background.attach_ms(backgroundMs, callback, this);
    }
    return true;
}

bool SysTemperature::loop(int32_t time)
{
    if (_background.active())
    {
        // when background-interrupt reading is active, loop returns true one time following a successful read
        bool pulse = _pulse;
        _pulse = false;
        return pulse;
    }
    return read();
}

bool SysTemperature::read()
{
    if (_readyPin != -1 && digitalRead(_readyPin) != LOW)
    {
        // ready pin is wired, and data is not available
        return false;
    }

    auto valueC = _max31856.readThermocoupleTemperature();
    output = (valueC * (212 - 32)) / 100 + 32;
    return true;
}

void SysTemperature::callback(SysTemperature *self)
{
    if (self->read())
    {
        // cause next loop to return true one time
        self->_pulse = true;
    }
}

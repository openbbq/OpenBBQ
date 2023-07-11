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

    uint8_t f = _max31856.readFault();
    if (f & MAX31856_FAULT_OPEN)
    {
        fault = "open";
    }
    else if (f & MAX31856_FAULT_OVUV)
    {
        fault = "volts";
    }
    else if (f & (MAX31856_FAULT_TCRANGE | MAX31856_FAULT_CJRANGE))
    {
        fault = "range";
    }
    else if (f & (MAX31856_FAULT_TCHIGH | MAX31856_FAULT_CJHIGH))
    {
        fault = "high";
    }
    else if (f & (MAX31856_FAULT_TCLOW | MAX31856_FAULT_CJLOW))
    {
        fault = "low";
    }
    else
    {
        fault = "";
    }

    faults = f;
    if (f==0)
    {
        auto valueC = _max31856.readThermocoupleTemperature();
        output = (valueC * (212 - 32)) / 100 + 32;
    }
    return true;
}

#define MAX31856_FAULT_CJRANGE \
    0x80 ///< Fault status Cold Junction Out-of-Range flag
#define MAX31856_FAULT_TCRANGE \
    0x40 ///< Fault status Thermocouple Out-of-Range flag
#define MAX31856_FAULT_CJHIGH \
    0x20                          ///< Fault status Cold-Junction High Fault flag
#define MAX31856_FAULT_CJLOW 0x10 ///< Fault status Cold-Junction Low Fault flag
#define MAX31856_FAULT_TCHIGH \
    0x08 ///< Fault status Thermocouple Temperature High Fault flag
#define MAX31856_FAULT_TCLOW \
    0x04 ///< Fault status Thermocouple Temperature Low Fault flag
#define MAX31856_FAULT_OVUV \
    0x02 ///< Fault status Overvoltage or Undervoltage Input Fault flag
#define MAX31856_FAULT_OPEN \
    0x01 ///< Fault

void SysTemperature::callback(SysTemperature *self)
{
    if (self->read())
    {
        // cause next loop to return true one time
        self->_pulse = true;
    }
}

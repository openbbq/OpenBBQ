
#include "ControlPID.h"

ControlPID::ControlPID() : _alpha(this)
{
    ConfigObject::add("SPA", &_setpoint.alpha);
    ConfigObject::add("band1", &band1);
    ConfigObject::add("Kp1", &Kp1);
    ConfigObject::add("Ki1", &Ki1);
    ConfigObject::add("Kd1", &Kd1);
    ConfigObject::add("band2", &band2);
    ConfigObject::add("Kp2", &Kp2);
    ConfigObject::add("Ki2", &Ki2);
    ConfigObject::add("Kd2", &Kd2);
    ConfigObject::add("ALPHA", &_alpha);
    ConfigObject::add("IMAX", &_integralMax);
    ConfigObject::add("IMIN", &_integralMin);
}

bool ControlPID::begin(int32_t periodMs)
{
    return _period.begin(periodMs, true) &&
           _setpoint.begin(1000) &&
           _derivativeFilters[0].begin(1000) &&
           _derivativeFilters[1].begin(1000) &&
           _derivativeFilters[2].begin(1000);
}

bool ControlPID::loop(int32_t timeMs)
{
    if (!_period.loop(timeMs))
    {
        return false;
    }

    _setpoint.loop(timeMs);

    float error = _setpoint.output - _processValue;

    // gain scheduling
    // 0 : at or below band1
    // 1 : at or above band2
    // 0->1 : band1->band2
    float gain = 0;
    if (band2 != 0 && fabs(band2 - band1) > .1)
    {
        gain = constrain((fabs(error) - band1) / (band2 - band1), 0, 1);
    }
    float Kp = gain * Kp2 + (1 - gain) * Kp1;
    float Ki = gain * Ki2 + (1 - gain) * Ki1;
    float Kd = gain * Kd2 + (1 - gain) * Kd1;

    if (_hasLast == false)
    {
        _proportional = Kp * error;
        _integral = 0;   // no time delta + no integral history
        _derivative = 0; // no error delta / no time delta
        for (int i = 0; i != 3; i++)
        {
            _derivativeFilters[i].signal = 0;
            _derivativeFilters[i].loop(timeMs);
        }
    }
    else
    {
        // fixed period --- using constant time delta for simplicity.
        float timeDelta = _period.ms() / 60000.0;
        float errorDelta = error - _errorLast;

        _proportional = Kp * error;
        _integral = constrain(Ki * (error * timeDelta) + _integralLast, float(_integralMin), float(_integralMax));
        _derivative = Kd * errorDelta / timeDelta;

        // low-pass filter on derivative
        for (int i = 0; i != 3; i++)
        {
            _derivativeFilters[i].signal = _derivative.value();
            _derivativeFilters[i].loop(timeMs);
            _derivative = _derivativeFilters[i].output.value();
        }
    }

    _out = _proportional + _integral + _derivative;

    _hasLast = true;
    _errorLast = error;
    _integralLast = _integral;

    return true;
}

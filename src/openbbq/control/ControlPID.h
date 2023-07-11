
#pragma once

#include <functional>

#include <openbbq/control/ControlSignal.h>
#include "ControlPeriod.h"
#include "ControlLowPassFilter.h"

#include <openbbq/config/Config.h>

class ControlPID : public ConfigObject
{
public:
    ControlPID();

    bool begin(int32_t periodMs);
    bool loop(int32_t timeMs);

    // input
    ControlLowPassFilter _setpoint;
    WithConnect<ControlSignal<float>> _processValue = 0;
    WithConnect<ControlSignal<int>> _processValueFaults = 0;

    // parameters
    WithConfig<ControlSignal<float>> band1 = 0;
    WithConfig<ControlSignal<float>> Kp1 = 0;
    WithConfig<ControlSignal<float>> Ki1 = 0;
    WithConfig<ControlSignal<float>> Kd1 = 0;

    WithConfig<ControlSignal<float>> band2 = 0;
    WithConfig<ControlSignal<float>> Kp2 = 0;
    WithConfig<ControlSignal<float>> Ki2 = 0;
    WithConfig<ControlSignal<float>> Kd2 = 0;

    WithConfig<ControlSignal<float>> _integralMax = 0;
    WithConfig<ControlSignal<float>> _integralMin = 0;

    // results
    ControlSignal<float> _proportional = 0;
    ControlSignal<float> _integral = 0;
    ControlSignal<float> _derivative = 0;
    ControlSignal<float> _out = 0;

    class Alpha : public WithRange<WithConfig<ControlSignal<float>>>
    {
        typedef WithRange<WithConfig<ControlSignal<float>>> Base;

    public:
        Alpha(ControlPID *pid) : Base(0, 0, 100), _pid(pid) {}

        void value(const float &t) override
        {
            Base::value(t);
            _pid->_derivativeFilters[0].alpha = Base::value();
            _pid->_derivativeFilters[1].alpha = Base::value();
            _pid->_derivativeFilters[2].alpha = Base::value();
        }

    private:
        ControlPID *_pid;
    };

    Alpha _alpha;

private:
    bool _hasLast = false;   // for initial loop, or reset
    float _errorLast = 0;    // for derivative
    float _integralLast = 0; // for integral

    ControlPeriod _period;
    ControlLowPassFilter _derivativeFilters[3];
};

#pragma once

class ControlPeriod
{
public:
    int32_t ms() { return _ms; }

    bool begin(int32_t ms, bool fixed = false)
    {
        _ms = ms;
        _fixed = fixed;
        _first = true;
        _next = 0;
        return true;
    }

    bool loop(int32_t time)
    {
        if (_first)
        {
            _first = false;
            _next = time + _ms;
            return true;
        }
        if (time >= _next)
        {
            if (_fixed)
            {
                _next = _next + _ms;
            }
            else
            {
                _next = time + _ms;
            }
            return true;
        }
        return false;
    }

private:
    int32_t _ms = 100;
    bool _fixed = false;
    bool _first = true;
    int32_t _next = 0;
};


#pragma once

#include <Arduino.h>
#include <Adafruit_MAX31856.h>
#include <Ticker.h>

#include <openbbq/control/ControlSignal.h>

class SysTemperature
{
public:
    SysTemperature(int8_t csPin, int8_t readyPin = -1)
        : _max31856(csPin),
          _readyPin(readyPin)
    {
    }

    bool begin(int32_t backgroundMs = 0);
    bool loop(int32_t time);

    bool read();

    ControlSignal<float> output = 0;
    
    // native device bits
    ControlSignal<int> faults = {0};

    // most significant short error
    ControlSignal<String> fault = {""};


private:
    static void callback(SysTemperature *self);

    // hardware
    Adafruit_MAX31856 _max31856;
    int8_t _readyPin = -1;

    // when background reading enabled
    Ticker _background;
    bool _pulse = false;
};

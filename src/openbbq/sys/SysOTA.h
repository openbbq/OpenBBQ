#pragma once 

#include <ArduinoOTA.h>

class SysOTA {
public:
    bool begin();
    bool loop();
};


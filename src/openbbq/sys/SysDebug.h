
#pragma once

#include <RemoteDebug.h>

extern RemoteDebug Debug;

class SysDebug
{
public:
    bool begin();
    bool loop();
};

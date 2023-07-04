#include "SysDebug.h"

RemoteDebug Debug;

bool SysDebug::begin()
{
    Debug.begin("openbbq");
    Debug.showColors(true);
    return true;
}

bool SysDebug::loop()
{
    Debug.handle();
    return true;
}

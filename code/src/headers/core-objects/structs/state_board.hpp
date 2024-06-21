#pragma once

#include "iinstance.h"
#include "iwindow.h"
#include "idevice.h"

namespace tlr
{

struct StateBoard
{
    static IInstance* instance;
    static IWindow*   window;
    static IDevice*   device;
};

} // namespace tlr
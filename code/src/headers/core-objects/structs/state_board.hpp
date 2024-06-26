#pragma once

#include "iinstance.h"
#include "iwindow.h"
#include "idevice.h"
#include "iswapchain.h"

namespace tlr
{

struct StateBoard
{
    static IInstance*  instance;
    static IWindow*    window;
    static IDevice*    device;
    static ISwapchain* swapchain;
};

} // namespace tlr
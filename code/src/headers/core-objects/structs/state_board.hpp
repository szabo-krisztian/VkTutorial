#pragma once

#include "iinstance.h"
#include "iwindow.h"

namespace tlr
{

struct StateBoard
{
    static IInstance* instance;
    static IWindow*   window;
};

} // namespace tlr
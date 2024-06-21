#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace tlr
{

class IWindow
{
public:
    virtual ~IWindow() = default;

    virtual const VkSurfaceKHR& GetSurface()     const = 0;
    virtual       GLFWwindow*   GetWindow()      const = 0;
    virtual       bool          IsWindowActive() const = 0; 
};

} // namespace tlr
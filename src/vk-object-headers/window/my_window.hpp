#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "my_instance.hpp"

namespace tlr
{

class MyWindow
{
public:
    MyWindow(MyInstance& instance);
    ~MyWindow();

    MyWindow(const MyWindow&) = delete;
    MyWindow operator=(const MyWindow&) = delete;

    const VkSurfaceKHR& GetSurface() const;
    GLFWwindow*         GetWindow();
    bool                IsWindowActive();
private:
    const uint32_t M_WIDTH = 800;
    const uint32_t M_HEIGHT = 600;

    MyInstance& mInstance;
    VkSurfaceKHR      mSurface;
    GLFWwindow*       mWindow;

    void InitWindow();
};

} // namespace tlr
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

namespace vlk
{
    
class VlkWindow 
{
public:
    VlkWindow(int width, int height, std::string windowName);
    ~VlkWindow();

    VlkWindow(const VlkWindow &) = delete;
    VlkWindow &operator=(const VlkWindow &) = delete;

    bool ShouldClose();

    void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
    GLFWwindow* mWindow;
    const int   mWidth;
    const int   mHeight;
    std::string mWindowName;

    void InitWindow();
};

} // namespace vlk
#pragma once

#include "my_instance.hpp"

namespace tlr
{

class MyVulkanWindow
{
public:
    MyVulkanWindow(MyVulkanInstance& instance);
    ~MyVulkanWindow();

    MyVulkanWindow(const MyVulkanWindow&) = delete;
    MyVulkanWindow operator=(const MyVulkanWindow&) = delete;

    const VkSurfaceKHR& GetSurface() const;
    GLFWwindow* GetWindow();
    bool IsWindowActive();
private:
    const uint32_t M_WIDTH = 800;
    const uint32_t M_HEIGHT = 600;
    MyVulkanInstance& mInstance;
    VkSurfaceKHR mSurface;
    GLFWwindow* mWindow;

    void InitWindow();
};

} // namespace tlr
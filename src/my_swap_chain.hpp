#pragma once

#include "my_window.hpp"
#include "my_device.hpp"

#include <cstdint>
#include <limits>
#include <algorithm>

namespace tlr
{

class MyVulkanSwapChain
{
public:
    MyVulkanSwapChain(MyVulkanWindow& window, MyVulkanDevice& device);
    ~MyVulkanSwapChain();

    MyVulkanSwapChain(const MyVulkanSwapChain&) = delete;
    MyVulkanSwapChain operator=(const MyVulkanSwapChain&) = delete;

private:
    MyVulkanWindow& mWindow;
    MyVulkanDevice& mDevice;
    VkSwapchainKHR mSwapChain;

    VkResult           InitSwapChain();
    VkSurfaceFormatKHR GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR   GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D         GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities);
};

} // namespace tlr
#pragma once

#include "swapchain.hpp"
#include "device.hpp"
#include "physical_device.hpp"
#include "toolset.hpp"

#include <iostream>
#include <limits>
#include <algorithm>

namespace tlr
{
    
class SwapchainBuilder
{
public:
    SwapchainBuilder(GLFWwindow* window, const VkSurfaceKHR& surface, const PhysicalDevice& physicalDevice, const Device& device);

    Swapchain         Build();
    SwapchainBuilder& SetDesiredFormat(VkFormat format);
    SwapchainBuilder& SetDesiredColorSpace(VkColorSpaceKHR colorSpace);
    SwapchainBuilder& SetDesiredPresentMode(VkPresentModeKHR presentMode);
    SwapchainBuilder& SetDesiredExtent(uint32_t width, uint32_t height);
    SwapchainBuilder& SetDesiredImageCount(uint32_t imageCount);
    SwapchainBuilder& SetImageFlags(VkImageUsageFlags usageFlags);
    SwapchainBuilder& SetDesiredArrayLayerCount(uint32_t arrayLayerCount);

private:

    struct SwapchainInfo
    {
        GLFWwindow* window = nullptr;
        VkSurfaceKHR surface{VK_NULL_HANDLE};
        PhysicalDevice physicalDevice;
        VkDevice device;
        VkFormat desiredFormat = VK_FORMAT_B8G8R8A8_SRGB;
        VkColorSpaceKHR desiredColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        uint32_t desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        uint32_t desiredWidth = 1000;
        uint32_t desiredHeight = 800;
        uint32_t imageCount = 0;
        VkSwapchainCreateFlagBitsKHR createFlags = static_cast<VkSwapchainCreateFlagBitsKHR>(0);
        uint32_t arrayLayerCount = 1;
        VkImageUsageFlags imageUsageFlags{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
        uint32_t graphicsQueueIndex = 0;
        uint32_t presentQueueIndex = 0;
        VkSurfaceTransformFlagBitsKHR preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(0);
    } _info;

    VkSurfaceFormatKHR GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR   GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D         GetSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

} // namespace tlr
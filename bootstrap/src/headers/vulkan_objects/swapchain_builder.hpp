#pragma once

#include "swapchain.hpp"
#include "device.hpp"
#include "physical_device.hpp"
#include "toolset.hpp"

namespace tlr
{
    
class SwapchainBuilder
{
public:
    SwapchainBuilder(const PhysicalDevice& physicalDevice, const Device& device, const VkSurfaceKHR& surface);

    Swapchain         Build();
    SwapchainBuilder& SetDesiredExtent(uint32_t width, uint32_t height);
    SwapchainBuilder& SetDesiredFormat(VkFormat format);
    SwapchainBuilder& SetDesiredColorSpace(VkColorSpaceKHR colorSpace);
    SwapchainBuilder& SetDesiredPresentMode(VkPresentModeKHR presentMode);
    SwapchainBuilder& SetImageFlags(VkImageUsageFlags usageFlags);
    SwapchainBuilder& SetImageLayerCount(uint32_t arrayLayerCount);

private:
    struct SwapchainInfo
    {
        VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
        VkDevice device{VK_NULL_HANDLE};
        VkSwapchainCreateFlagBitsKHR createFlags = static_cast<VkSwapchainCreateFlagBitsKHR>(0);
        VkSurfaceKHR surface{VK_NULL_HANDLE};
        VkFormat desiredFormat;
        VkColorSpaceKHR desiredColorSpace;
        uint32_t desiredWidth = 800;
        uint32_t desiredHeight = 600;
        VkPresentModeKHR presentMode;
        uint32_t arrayLayerCount = 1;
        uint32_t minImageCount = 0;
        uint32_t requiredMinImageCount = 0;
        VkImageUsageFlags imageUsageFlags{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
        uint32_t graphicsQueueIndex = 0;
        uint32_t presentQueueIndex = 0;
        VkSurfaceTransformFlagBitsKHR preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(0);
    } _info;

    VkSurfaceFormatKHR GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR   GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D         GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities);
};

} // namespace tlr
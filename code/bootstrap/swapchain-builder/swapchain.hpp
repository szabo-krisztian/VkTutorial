#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace tlr
{

struct Swapchain
{
    VkDevice                 device{VK_NULL_HANDLE};
    VkSwapchainKHR           swapchain{VK_NULL_HANDLE};
    uint32_t                 imageCount = 0;
    VkFormat                 imageFormat{VK_FORMAT_UNDEFINED};
    VkColorSpaceKHR          colorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    VkImageUsageFlags        imageUsageFlags = 0;
    VkExtent2D               extent{0, 0};
    VkPresentModeKHR         presentMode{VK_PRESENT_MODE_IMMEDIATE_KHR};
    std::vector<VkImage>     images;
    std::vector<VkImageView> imageViews;

    operator VkSwapchainKHR() const
    {
        return swapchain;
    }
};

} // namespace tlr
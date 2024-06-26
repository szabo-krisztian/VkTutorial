#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace tlr
{

class ISwapchain
{
public:
    virtual ~ISwapchain() = default;

    virtual const VkSwapchainKHR&             Get()             const = 0;
    virtual const VkFormat&                   GetFormat()       const = 0;
    virtual const VkExtent2D&                 GetExtent()       const = 0;
    virtual const std::vector<VkImageView>&   GetImageViews()   const = 0;
};

} // namespace tlr
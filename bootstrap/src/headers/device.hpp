#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "physical_device.hpp"

namespace tlr
{

struct Device
{
    VkDevice device;
    PhysicalDevice physicalDevice;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    std::vector<VkQueueFamilyProperties> queueFamilies;

    operator VkDevice() const
    {
        return device;
    }
};

} // namespace tlr
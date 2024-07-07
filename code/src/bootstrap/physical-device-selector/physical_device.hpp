#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

#include "queue_family_indices.hpp"
#include "swapchain_support_details.hpp"

namespace tlr
{

struct PhysicalDevice
{
    std::string name;
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    std::vector<const char*> extensions;
    VkPhysicalDeviceFeatures features{};
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceMemoryProperties memoryProperties{};

    QueueFamilyIndices familyIndices;
    SwapchainSupportDetails swapchainSupportDetails;

    operator VkPhysicalDevice() const
    {
        return physicalDevice;
    }
};

} // namespace tlr
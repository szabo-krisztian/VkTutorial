#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "queue_family_indices.hpp"
#include "swapchain_support_details.hpp"
#include "physical_device.hpp"

namespace tlr
{

class PhysicalDeviceSelector
{
public:
    PhysicalDeviceSelector(VkInstance intance, VkSurfaceKHR surface);

    PhysicalDevice          Select();
    PhysicalDeviceSelector& EnableDedicatedGPU();
    PhysicalDeviceSelector& SetExtensions(const std::vector<const char*>& extensions);
    
private:
    struct PhysicalDeviceInfo
    {
        std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        bool                     isDedicatedGPU = false;
        bool                     isGraphicsEnabled = false;
        bool                     isPresentEnabled = false;
    } _info;

    VkInstance   _instance{ VK_NULL_HANDLE };
    VkSurfaceKHR _surface{ VK_NULL_HANDLE };

    std::vector<VkPhysicalDevice> GetPhysicalDevices();
    bool                          IsDeviceSuitable(const VkPhysicalDevice& device);
    QueueFamilyIndices            GetQueueFamilyIndices(const VkPhysicalDevice& physicalDevice);
    bool                          IsDeviceExtensionsSupported(const VkPhysicalDevice& physicalDevice);
    bool                          IsDedicatedDevice(const VkPhysicalDevice& device);
    SwapchainSupportDetails       GetSwapchainSupport(const VkPhysicalDevice& physicalDevice);

};

} // namespace tlr
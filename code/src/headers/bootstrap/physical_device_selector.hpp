#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>

#include "toolset.hpp"
#include "queue_family_indices.hpp"

namespace tlr
{

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


struct PhysicalDevice
{
    std::string name;
    VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkPhysicalDeviceFeatures features{};
    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceMemoryProperties memoryProperties{};

    QueueFamilyIndices familyIndices;
    SwapchainSupportDetails swapchainSupportDetails;

    operator VkPhysicalDevice() const;
};

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
        bool isDedicatedGPU = false;
    } info;
    
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
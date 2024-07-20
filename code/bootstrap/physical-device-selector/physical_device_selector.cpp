/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#include "physical_device_selector.hpp"

namespace tlr
{

PhysicalDeviceSelector::PhysicalDeviceSelector(VkInstance instance, VkSurfaceKHR surface) : _instance{instance}, _surface{surface} {}

PhysicalDeviceSelector& PhysicalDeviceSelector::EnableDedicatedGPU()
{
    _info.isDedicatedGPU = true;
    return *this;
}

PhysicalDeviceSelector& PhysicalDeviceSelector::SetExtensions(const std::vector<const char*>& extensions)
{
    _info.extensions = extensions;
    return *this;
}

PhysicalDevice PhysicalDeviceSelector::Select()
{
    PhysicalDevice deviceInfo{};

    std::vector<VkPhysicalDevice> physicalDevices = GetPhysicalDevices();
    bool suitable = false;
    for (const auto& device : physicalDevices)
    {
        if (IsDeviceSuitable(device))
        {
            suitable = true;
            deviceInfo.physicalDevice = device;
            break;
        }
    }

    if (!suitable)
    {
        throw std::runtime_error("failed to find suitable physical device!");
    }

    vkGetPhysicalDeviceProperties(deviceInfo.physicalDevice, &deviceInfo.properties);
    vkGetPhysicalDeviceMemoryProperties(deviceInfo.physicalDevice, &deviceInfo.memoryProperties);

    deviceInfo.name = deviceInfo.properties.deviceName;
    deviceInfo.surface = _surface;
    deviceInfo.extensions = _info.extensions;
    deviceInfo.familyIndices = GetQueueFamilyIndices(deviceInfo.physicalDevice);
    deviceInfo.swapchainSupportDetails = GetSwapchainSupport(deviceInfo.physicalDevice);
    
    return deviceInfo;
}

std::vector<VkPhysicalDevice> PhysicalDeviceSelector::GetPhysicalDevices()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(_instance, &count, nullptr);

    if (count == 0)
    {
        throw std::runtime_error("failed to find physical devices, probably invalid instance value!");
    }

    std::vector<VkPhysicalDevice> devices;
    devices.resize(count);
    vkEnumeratePhysicalDevices(_instance, &count, devices.data());
    return devices;
}

bool PhysicalDeviceSelector::IsDeviceSuitable(const VkPhysicalDevice& device)
{
    QueueFamilyIndices indices = GetQueueFamilyIndices(device);

    if (!IsDeviceExtensionsSupported(device))
    {
        return false;
    }
    if (_info.isDedicatedGPU && !IsDedicatedDevice(device))
    {
        return false;
    }

    bool isSwapChainAdequate = false;
    SwapchainSupportDetails swapChainSupport = GetSwapchainSupport(device);
    isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    
    return indices.IsComplete() && isSwapChainAdequate;
}

QueueFamilyIndices PhysicalDeviceSelector::GetQueueFamilyIndices(const VkPhysicalDevice& physicalDevice)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    VkBool32 isPresentSupported = false;
    for (const auto& queueFamily : queueFamilies)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &isPresentSupported);
        
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        if (isPresentSupported)
        {
            indices.presentFamily = i;
        }
        if (indices.IsComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

bool PhysicalDeviceSelector::IsDeviceExtensionsSupported(const VkPhysicalDevice& physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(_info.extensions.begin(), _info.extensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool PhysicalDeviceSelector::IsDedicatedDevice(const VkPhysicalDevice& device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    return (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
}

SwapchainSupportDetails PhysicalDeviceSelector::GetSwapchainSupport(const VkPhysicalDevice& physicalDevice)
{
    SwapchainSupportDetails details = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,  _surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

} // namespace tlr
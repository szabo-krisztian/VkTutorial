/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#include "device_builder.hpp"

#include "toolset.hpp"

namespace tlr
{

DeviceBuilder::DeviceBuilder(PhysicalDevice physicalDevice) : _physicalDevice{ std::move(physicalDevice) } { }

Device DeviceBuilder::Build()
{
    Device device{};
    device.physicalDevice = _physicalDevice;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { _physicalDevice.familyIndices.graphicsFamily.value(), _physicalDevice.familyIndices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (const auto& queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};   
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    
    if (_info.isValidationLayersEnabled)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(_info.layers.size());
        createInfo.ppEnabledLayerNames = _info.layers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(_physicalDevice.extensions.size());
    createInfo.ppEnabledExtensionNames = _physicalDevice.extensions.data();
    createInfo.pEnabledFeatures = nullptr;
    VK_CHECK_RESULT(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &device.device));

    device.queues.graphicsFamily = _physicalDevice.familyIndices.graphicsFamily.value();
    vkGetDeviceQueue(device, _physicalDevice.familyIndices.graphicsFamily.value(), 0, &device.queues.graphics);
    device.queues.presentFamily = _physicalDevice.familyIndices.presentFamily.value();
    vkGetDeviceQueue(device, _physicalDevice.familyIndices.presentFamily.value(), 0, &device.queues.present);

    return device;
}

DeviceBuilder& DeviceBuilder::EnableValidationLayers()
{
    _info.isValidationLayersEnabled = true;
    return *this;
}

} // namespace tlr
/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * - Sascha Willems' Vulkan Buffer Class
 *   Copyright (c) 2016 Sascha Willems
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#pragma once

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "physical_device.hpp"
#include "buffer.hpp"

namespace tlr
{

struct Device
{
    VkDevice                             device;
    PhysicalDevice                       physicalDevice;
    std::vector<VkQueueFamilyProperties> queueFamilies;

    operator VkDevice() const
    {
        return device;
    }

    struct
	{
        uint32_t graphicsFamily;
		VkQueue graphics;
        uint32_t presentFamily;
		VkQueue present;
	} queues;

    uint32_t GetMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer *buffer, VkDeviceSize size, void *data = nullptr);

};

} // namespace tlr
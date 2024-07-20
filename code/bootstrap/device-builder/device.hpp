/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "physical_device.hpp"

namespace tlr
{

struct Device
{
    VkDevice                             device;
    PhysicalDevice                       physicalDevice;
    VkSurfaceKHR                         surface{VK_NULL_HANDLE};
    std::vector<VkQueueFamilyProperties> queueFamilies;

    operator VkDevice() const
    {
        return device;
    }
};

} // namespace tlr
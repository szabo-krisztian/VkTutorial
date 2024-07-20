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

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "queue_family_indices.hpp"
#include "swapchain_support_details.hpp"

namespace tlr
{

struct PhysicalDevice
{
    std::string                      name;
    VkPhysicalDevice                 physicalDevice{ VK_NULL_HANDLE };
    VkSurfaceKHR                     surface{ VK_NULL_HANDLE };
    std::vector<const char*>         extensions;
    VkPhysicalDeviceFeatures         features{};
    VkPhysicalDeviceProperties       properties{};
    VkPhysicalDeviceMemoryProperties memoryProperties{};
    QueueFamilyIndices               familyIndices;
    SwapchainSupportDetails          swapchainSupportDetails;

    operator VkPhysicalDevice() const
    {
        return physicalDevice;
    }
};

} // namespace tlr
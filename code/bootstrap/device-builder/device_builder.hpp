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

#include <iostream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "physical_device.hpp"
#include "device.hpp"

namespace tlr
{

class DeviceBuilder
{
public:
    DeviceBuilder(PhysicalDevice physicalDevice);

    Device         Build();
    DeviceBuilder& EnableValidationLayers();

private:
    struct DeviceInfo
    {
        std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
        bool isValidationLayersEnabled = false;
    } _info;
    PhysicalDevice _physicalDevice;
};

} // namespace tlr
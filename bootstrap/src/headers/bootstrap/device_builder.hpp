#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <set>

#include "toolset.hpp"
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
#pragma once

#include "default_instance_builder.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

#include <GLFW/glfw3.h>

namespace tlr
{

class DebugInstanceBuilder : public DefaultInstanceBuilder
{
public:
    DebugInstanceBuilder();
    ~DebugInstanceBuilder();

    DebugInstanceBuilder(const DebugInstanceBuilder&) = delete;
    DebugInstanceBuilder operator=(const DebugInstanceBuilder&) = delete;

    void SetVkInstanceCreateInfo() override;
private:
    const std::vector<const char*>     M_VALIDATION_LAYERS;
    const std::vector<const char*>     M_EXTENSIONS;
    
    VkDebugUtilsMessengerCreateInfoEXT mDebugCreateInfo;

    bool                               AreValidationLayersSupported();
    bool                               IsValidationLayerSupported(const char* layerName, const std::vector<VkLayerProperties>& availableLayerProperties);
    void                               SetVkDebugUtilsMessengerCreateInfoEXT();
    std::vector<const char*>           GetRequiredExtensions();
};

} // namespace tlr
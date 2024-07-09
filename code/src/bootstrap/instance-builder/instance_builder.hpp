#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <iostream>

#include "instance.hpp"

namespace tlr
{

class InstanceBuilder
{
public:
    InstanceBuilder();

    Instance                 Build();
    InstanceBuilder&         SetApplicationName(const char* applicationName);
    InstanceBuilder&         SetApplicationVersion(uint32_t applicationVersion);
    InstanceBuilder&         SetEngineName(const char* engineName);
    InstanceBuilder&         SetEngineVersion(uint32_t engineVersion);
    InstanceBuilder&         SetApiVersion(uint32_t apiVersion);
    InstanceBuilder&         RequestDefaultLayers();
    InstanceBuilder&         RequestDefaultExtensions();
    InstanceBuilder&         UseDebugMessenger();
    
private:
    struct InstanceInfo
    {
        const char* applicationName = nullptr;
        uint32_t applicationVersion = 0;
        const char* engineName = nullptr;
        uint32_t engineVersion = 0;
        uint32_t apiVersion = 0;

        std::vector<const char*> defaultLayers;
        std::vector<const char*> defaultExtensions;

        uint32_t debugMessageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        uint32_t debugMessageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        bool isDebugMessengerRequested = false;
        bool isDefaultLayersRequested = false;
        bool isDefaultExtensionsRequested = false;
    } info;

    bool AreValidationLayersSupported();
    bool IsValidationLayerSupported(const char *layerName, const std::vector<VkLayerProperties> &availableLayerProperties);
    std::vector<const char*> GetDefaultExtensions();
};

void DestroyDebugMessenger(const VkInstance& instance, const VkDebugUtilsMessengerEXT& messenger);

} // namespace tlr
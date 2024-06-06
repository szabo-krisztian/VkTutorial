#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <cstring>
#include <stdexcept>

namespace tlr
{

class MyVulkanInstance
{
public:
    MyVulkanInstance();
    ~MyVulkanInstance();

    MyVulkanInstance(const MyVulkanInstance&) = delete;
    MyVulkanInstance operator=(const MyVulkanInstance&) = delete;

    VkInstance Get() const;
private:
    const std::vector<const char*> M_VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
    const bool                     M_ENABLE_VALIDATION_LAYERS = false;
#else
    const bool                     M_ENABLE_VALIDATION_LAYERS = true;
#endif

    VkInstance                     mInstance;
    VkDebugUtilsMessengerEXT       mDebugMessenger;

    void                     InitVulkanInstance();
    void                     CreateAppInfo(VkApplicationInfo& appInfo);
    bool                     AreValidationLayersSupported();
    bool                     IsValidationLayerSupported(const char* layerName, const std::vector<VkLayerProperties>& availableLayerProperties);
    std::vector<const char*> GetRequiredExtensions();
    void                     CreateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void                     InitDebugMessengerInstance();
};

} // namespace tlr

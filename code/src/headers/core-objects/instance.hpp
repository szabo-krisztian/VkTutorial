#pragma once

#include <GLFW/glfw3.h>

#include "iinstance.h"

#include <iostream>
#include <vector>
#include <stdexcept>

namespace tlr
{

class Instance : public IInstance
{
public:
    Instance();
    ~Instance();

    Instance(const Instance&) = delete;
    Instance operator=(const Instance&) = delete;

    const VkInstance& Get() const override;

private:
    const std::vector<const char*>     M_VALIDATION_LAYERS;
    const std::vector<const char*>     M_EXTENSIONS;

    VkInstance                         mInstance;
    VkApplicationInfo                  mAppCreateInfo;
    VkDebugUtilsMessengerCreateInfoEXT mDebugMessengerCreateInfo;
    VkInstanceCreateInfo               mInstanceCreateInfo;

    void                               SetVkApplicationInfo();
    void                               SetVkDebugUtilsMessengerCreateInfoEXT();
    bool                               AreValidationLayersSupported();
    bool                               IsValidationLayerSupported(const char* layerName, const std::vector<VkLayerProperties>& availableLayerProperties);
    std::vector<const char*>           GetRequiredExtensions();
    void                               SetVkInstanceCreateInfo();
};

}; // namespace tlr
#include <cstring>

#include "instance_builder.hpp"
#include "toolset.hpp"

namespace tlr
{

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT  *pCallbackData,
    void *pUserData
)
{
    std::cerr << "-----------------------------------------" << std::endl << "Validation layer: " << pCallbackData->pMessage << std::endl << "-----------------------------------------" << std::endl << std::endl;
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT
(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks              *pAllocator,
    VkDebugUtilsMessengerEXT                 *pDebugMessenger
)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT
(
    VkInstance                  instance,
    VkDebugUtilsMessengerEXT    debugMessenger,
    const VkAllocationCallbacks *pAllocator
)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

InstanceBuilder::InstanceBuilder()
{
    _info.defaultLayers = { "VK_LAYER_KHRONOS_validation" };
    _info.defaultExtensions = GetDefaultExtensions();
}

Instance InstanceBuilder::Build()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = _info.applicationName;
    applicationInfo.applicationVersion = _info.applicationVersion;
    applicationInfo.pEngineName = _info.applicationName;
    applicationInfo.engineVersion = _info.engineVersion;
    applicationInfo.apiVersion = _info.apiVersion;

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCI = {};
    debugMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugMessengerCI.pNext = nullptr;
    debugMessengerCI.flags = 0;
    debugMessengerCI.messageSeverity = _info.debugMessageSeverity;
    debugMessengerCI.messageType = _info.debugMessageType;
    debugMessengerCI.pfnUserCallback = DebugCallback;
    debugMessengerCI.pUserData = nullptr;

    VkInstanceCreateInfo instanceCI = {};
    instanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCI.pNext = &debugMessengerCI;
    instanceCI.flags = 0;
    instanceCI.pApplicationInfo = &applicationInfo;
    
    instanceCI.enabledLayerCount = static_cast<uint32_t>(0);
    instanceCI.ppEnabledLayerNames = nullptr;
    if (_info.isDefaultLayersRequested)
    {
        instanceCI.enabledLayerCount = static_cast<uint32_t>(_info.defaultLayers.size());
        instanceCI.ppEnabledLayerNames = _info.defaultLayers.data();
    }

    instanceCI.enabledExtensionCount = static_cast<uint32_t>(0);
    instanceCI.ppEnabledExtensionNames = nullptr;
    if (_info.isDefaultExtensionsRequested)
    {
        instanceCI.enabledExtensionCount = static_cast<uint32_t>(_info.defaultExtensions.size());
        instanceCI.ppEnabledExtensionNames = _info.defaultExtensions.data();
    }

    Instance instance = {};

    VK_CHECK_RESULT(vkCreateInstance(&instanceCI, nullptr, &instance.instance));

    if (_info.isDebugMessengerRequested)
    {
        VK_CHECK_RESULT(CreateDebugUtilsMessengerEXT(instance, &debugMessengerCI, nullptr, &instance.debugMessenger));
    }
    
    return instance;
}

InstanceBuilder& InstanceBuilder::SetApplicationName(const char* applicationName)
{
    _info.applicationName = applicationName;
    return *this;
}

InstanceBuilder& InstanceBuilder::SetApplicationVersion(uint32_t applicationVersion)
{
    _info.applicationVersion = applicationVersion;
    return *this;
}

InstanceBuilder& InstanceBuilder::SetEngineName(const char* engineName)
{
    _info.engineName = engineName;
    return *this;
}

InstanceBuilder& InstanceBuilder::SetEngineVersion(uint32_t engineVersion)
{
    _info.engineVersion = engineVersion;
    return *this;
}

InstanceBuilder& InstanceBuilder::SetApiVersion(uint32_t apiVersion)
{
    _info.apiVersion = apiVersion;
    return *this;
}

InstanceBuilder& InstanceBuilder::RequestDefaultLayers()
{
    _info.isDefaultLayersRequested = true;
    if (!AreValidationLayersSupported())
    {
        throw std::runtime_error("default validation layers are not supported!");
    }
    return *this;
}

InstanceBuilder& InstanceBuilder::RequestDefaultExtensions()
{
    _info.isDefaultExtensionsRequested = true;
    return *this;
}

InstanceBuilder& InstanceBuilder::UseDebugMessenger()
{
    _info.isDebugMessengerRequested = true;
    return *this;
}

bool InstanceBuilder::AreValidationLayersSupported()
{
    uint32_t layersCount;
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
    std::vector<VkLayerProperties> availableLayerProperties(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, availableLayerProperties.data());

    for (const auto &layerName : _info.defaultLayers)
    {
        if (!IsValidationLayerSupported(layerName, availableLayerProperties))
        {
            return false;
        }
    }

    return true;
}

bool InstanceBuilder::IsValidationLayerSupported(const char *layerName, const std::vector<VkLayerProperties> &availableLayerProperties)
{
    for (const auto &layerProperty : availableLayerProperties)
    {
        if (strcmp(layerName, layerProperty.layerName) == 0)
        {
            return true;
        }
    }

    return false;
}

std::vector<const char *> InstanceBuilder::GetDefaultExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void DestroyDebugMessenger(const VkInstance& instance, const VkDebugUtilsMessengerEXT& messenger)
{
    DestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
}

} // namespace tlr
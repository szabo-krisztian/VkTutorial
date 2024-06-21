#include "instance.hpp"

namespace tlr
{

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cerr << "-----------------------------------------" << std::endl
                << "Validation layer: " << pCallbackData->pMessage << std::endl
                << "-----------------------------------------" << std::endl
                << std::endl;
    return VK_FALSE;
}

Instance::Instance() : M_VALIDATION_LAYERS({"VK_LAYER_KHRONOS_validation"}), M_EXTENSIONS(GetRequiredExtensions())
{
    if (!AreValidationLayersSupported())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    SetVkApplicationInfo();
    SetVkDebugUtilsMessengerCreateInfoEXT();
    SetVkInstanceCreateInfo();

     if (vkCreateInstance(&mInstanceCreateInfo, nullptr, &mInstance))
    {
        throw std::runtime_error("instance creation failure!");
    }
}

Instance::~Instance()
{
    vkDestroyInstance(mInstance, nullptr);
}

const VkInstance& Instance::Get() const
{
    return mInstance;
}

void Instance::SetVkApplicationInfo()
{
    mAppCreateInfo = {};
    mAppCreateInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    mAppCreateInfo.pNext = nullptr;
    mAppCreateInfo.pApplicationName = "Vulkan Application";
    mAppCreateInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    mAppCreateInfo.pEngineName = "No Engine";
    mAppCreateInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    mAppCreateInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);
}

void Instance::SetVkDebugUtilsMessengerCreateInfoEXT()
{
    mDebugMessengerCreateInfo = {};
    mDebugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    mDebugMessengerCreateInfo.pNext = nullptr;
    mDebugMessengerCreateInfo.flags = 0;
    mDebugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    mDebugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    mDebugMessengerCreateInfo.pfnUserCallback = DebugCallback;
    mDebugMessengerCreateInfo.pUserData = nullptr;
}

bool Instance::AreValidationLayersSupported()
{
    uint32_t layersCount;
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
    std::vector<VkLayerProperties> availableLayerProperties(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, availableLayerProperties.data());

    for (const auto &layerName : M_VALIDATION_LAYERS)
    {
        if (!IsValidationLayerSupported(layerName, availableLayerProperties))
        {
            return false;
        }
    }

    return true;
}

bool Instance::IsValidationLayerSupported(const char *layerName, const std::vector<VkLayerProperties> &availableLayerProperties)
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

std::vector<const char *> Instance::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

void Instance::SetVkInstanceCreateInfo()
{
    mInstanceCreateInfo = {};
    mInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    mInstanceCreateInfo.pNext = nullptr;
    mInstanceCreateInfo.flags = 0;
    mInstanceCreateInfo.pApplicationInfo = &mAppCreateInfo;
    mInstanceCreateInfo.enabledLayerCount = 0;
    mInstanceCreateInfo.ppEnabledLayerNames = nullptr;
    mInstanceCreateInfo.enabledExtensionCount = 0;
    mInstanceCreateInfo.ppEnabledExtensionNames = nullptr;
}

} // namespace tlr
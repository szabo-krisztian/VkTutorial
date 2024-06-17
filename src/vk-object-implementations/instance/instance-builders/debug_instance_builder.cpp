#include "debug_instance_builder.hpp"

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

DebugInstanceBuilder::DebugInstanceBuilder() : M_VALIDATION_LAYERS({"VK_LAYER_KHRONOS_validation"}), M_EXTENSIONS(GetRequiredExtensions())
{
    if (!AreValidationLayersSupported())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    SetVkDebugUtilsMessengerCreateInfoEXT();
}

DebugInstanceBuilder::~DebugInstanceBuilder() {}

void DebugInstanceBuilder::SetVkInstanceCreateInfo()
{
    mInstanceCreateInfo = {};
    mInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    mInstanceCreateInfo.pNext = &mDebugCreateInfo;
    mInstanceCreateInfo.flags = 0;
    mInstanceCreateInfo.pApplicationInfo = &mApplicationCreateInfo;
    mInstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(M_VALIDATION_LAYERS.size());
    mInstanceCreateInfo.ppEnabledLayerNames = M_VALIDATION_LAYERS.data();
    mInstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(M_EXTENSIONS.size());
    mInstanceCreateInfo.ppEnabledExtensionNames = M_EXTENSIONS.data();
}

void DebugInstanceBuilder::SetVkDebugUtilsMessengerCreateInfoEXT()
{
    mDebugCreateInfo = {};
    mDebugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    mDebugCreateInfo.pNext = nullptr;
    mDebugCreateInfo.flags = 0;
    mDebugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    mDebugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    mDebugCreateInfo.pfnUserCallback = DebugCallback;
    mDebugCreateInfo.pUserData = nullptr;
}

bool DebugInstanceBuilder::AreValidationLayersSupported()
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

bool DebugInstanceBuilder::IsValidationLayerSupported(const char *layerName, const std::vector<VkLayerProperties> &availableLayerProperties)
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

std::vector<const char *> DebugInstanceBuilder::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

} // namespace tlr
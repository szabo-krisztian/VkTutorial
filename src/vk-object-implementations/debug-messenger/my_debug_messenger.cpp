#include "my_debug_messenger.hpp"

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

MyDebugMessenger::MyDebugMessenger(MyInstance& instance) : mInstance(instance)
{
    SetVkDebugUtilsMessengerCreateInfoEXT();
    InitDebugMessengerInstance();
}

MyDebugMessenger::~MyDebugMessenger()
{
    DestroyDebugUtilsMessengerEXT(mInstance.Get(), mDebugMessenger, nullptr);
}

void MyDebugMessenger::SetVkDebugUtilsMessengerCreateInfoEXT()
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

void MyDebugMessenger::InitDebugMessengerInstance()
{
    if (CreateDebugUtilsMessengerEXT(mInstance.Get(), &mDebugMessengerCreateInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("debug messenger creation failure!");
    }
}

} // namespace tlr
#include "my_instance.hpp"

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

MyVulkanInstance::MyVulkanInstance()
{
    InitVulkanInstance();
    InitDebugMessengerInstance();
}

MyVulkanInstance::~MyVulkanInstance()
{
    DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
    vkDestroyInstance(mInstance, nullptr);
}

VkInstance MyVulkanInstance::Get()
{
    return mInstance;
}

void MyVulkanInstance::InitVulkanInstance()
{
    /*
    // Provided by VK_VERSION_1_0
    typedef struct VkInstanceCreateInfo {
        VkStructureType             sType;
        const void*                 pNext;
        VkInstanceCreateFlags       flags;
        const VkApplicationInfo*    pApplicationInfo;
        uint32_t                    enabledLayerCount;
        const char* const*          ppEnabledLayerNames;
        uint32_t                    enabledExtensionCount;
        const char* const*          ppEnabledExtensionNames;
    } VkInstanceCreateInfo;
    */

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    if (M_ENABLE_VALIDATION_LAYERS)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        CreateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        createInfo.pNext = nullptr;
    }

    createInfo.flags = 0;

    VkApplicationInfo appInfo = {};    
    CreateAppInfo(appInfo);
    createInfo.pApplicationInfo = &appInfo;

    if (M_ENABLE_VALIDATION_LAYERS && !AreValidationLayersSupported())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    if (M_ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(M_VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = M_VALIDATION_LAYERS.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }
    
    std::vector<const char*> extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
    {
        throw std::runtime_error("vulkan instance creation failure!");
    }
}

void MyVulkanInstance::CreateAppInfo(VkApplicationInfo& appInfo)
{
    /*
    // Provided by VK_VERSION_1_0
    typedef struct VkApplicationInfo {
        VkStructureType    sType;
        const void*        pNext;
        const char*        pApplicationName;
        uint32_t           applicationVersion;
        const char*        pEngineName;
        uint32_t           engineVersion;
        uint32_t           apiVersion;
    } VkApplicationInfo;
    */

    appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = nullptr;
    appInfo.pApplicationName = "Vulkan Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);
}

bool MyVulkanInstance::AreValidationLayersSupported()
{
    uint32_t layersCount;
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
    std::vector<VkLayerProperties> availableLayerProperties(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, availableLayerProperties.data());

    for (const auto& layerName : M_VALIDATION_LAYERS)
    {
        if (!IsValidationLayerSupported(layerName, availableLayerProperties))
        {
            return false;
        }
    }

    return true;
}

bool MyVulkanInstance::IsValidationLayerSupported(const char* layerName, const std::vector<VkLayerProperties>& availableLayerProperties)
{
    for (const auto& layerProperty : availableLayerProperties)
    {
        if (strcmp(layerName, layerProperty.layerName) == 0)
        {
            return true;
        }
    }
    
    return false;
}

std::vector<const char*> MyVulkanInstance::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (M_ENABLE_VALIDATION_LAYERS)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void MyVulkanInstance::CreateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &debugCreateInfo)
{
    /*
    // Provided by VK_EXT_debug_utils
    typedef struct VkDebugUtilsMessengerCreateInfoEXT {
        VkStructureType                         sType;
        const void*                             pNext;
        VkDebugUtilsMessengerCreateFlagsEXT     flags;
        VkDebugUtilsMessageSeverityFlagsEXT     messageSeverity;
        VkDebugUtilsMessageTypeFlagsEXT         messageType;
        PFN_vkDebugUtilsMessengerCallbackEXT    pfnUserCallback;
        void*                                   pUserData;
    } VkDebugUtilsMessengerCreateInfoEXT;
    */

    debugCreateInfo = {};
    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.pNext = nullptr;
    debugCreateInfo.flags = 0;
    debugCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = DebugCallback;
    debugCreateInfo.pUserData = nullptr;
}

void MyVulkanInstance::InitDebugMessengerInstance()
{
    if (!M_ENABLE_VALIDATION_LAYERS) { return; }
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    CreateDebugMessengerCreateInfo(debugCreateInfo);
    if (CreateDebugUtilsMessengerEXT(mInstance, &debugCreateInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("debug messenger creation failure!");
    }
}

} // namespace tlr
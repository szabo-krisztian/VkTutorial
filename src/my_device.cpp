#include "my_device.hpp"

namespace tlr
{

MyVulkanDevice::MyVulkanDevice(MyVulkanInstance& instance, MyVulkanWindow& window) : mInstance(instance), mWindow(window)
{
    mPhysicalDevice = GetPhysicalDevice();
    
    if (mPhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
    mFamilyIndices = GetQueueFamilyIndices(mPhysicalDevice);
    mSwapChainSupportDetails = GetSwapChainSupport(mPhysicalDevice);

    if (InitLogicalDevice() != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
    
    vkGetDeviceQueue(mLogicalDevice, mFamilyIndices.graphicsFamily.value(), 0, &mGraphicsQueue);
    vkGetDeviceQueue(mLogicalDevice, mFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
}

MyVulkanDevice::~MyVulkanDevice()
{
    vkDestroyDevice(mLogicalDevice, nullptr);
}

const VkPhysicalDevice& MyVulkanDevice::GetPhysical() const
{
    return mPhysicalDevice;
}

const VkDevice& MyVulkanDevice::GetLogical() const
{
    return mLogicalDevice;
}

const QueueFamilyIndices& MyVulkanDevice::GetQueueFamilyIndices() const
{
    return mFamilyIndices;
}

const SwapChainSupportDetails& MyVulkanDevice::GetSwapChainSupport() const
{
    return mSwapChainSupportDetails;
}

VkPhysicalDevice MyVulkanDevice::GetPhysicalDevice()
{
    std::vector<VkPhysicalDevice> physicalDevices = GetPhysicalDevices();
    for (const auto& device : physicalDevices)
    {
        if (IsPhysicalDeviceSuitable(device))
        {
            return device;
        }
    }

    return VK_NULL_HANDLE;
}

std::vector<VkPhysicalDevice> MyVulkanDevice::GetPhysicalDevices()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance.Get(), &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(mInstance.Get(), &physicalDeviceCount, physicalDevices.data());
    return physicalDevices;
}

bool MyVulkanDevice::IsPhysicalDeviceSuitable(const VkPhysicalDevice& physicalDevice)
{
    QueueFamilyIndices indices = GetQueueFamilyIndices(physicalDevice);
    
    if (!IsDeviceExtensionsSupported(physicalDevice))
    {
        return false;
    }

    bool isSwapChainAdequate = false;
    SwapChainSupportDetails swapChainSupport = GetSwapChainSupport(physicalDevice);
    isSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();

    return indices.IsComplete() && isSwapChainAdequate;
}

SwapChainSupportDetails MyVulkanDevice::GetSwapChainSupport(const VkPhysicalDevice& physicalDevice)
{
    SwapChainSupportDetails details = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, mWindow.GetSurface(), &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mWindow.GetSurface(), &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, mWindow.GetSurface(), &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, mWindow.GetSurface(), &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,  mWindow.GetSurface(), &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool MyVulkanDevice::IsDeviceExtensionsSupported(const VkPhysicalDevice& physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(M_DEVICE_EXTENSIONS.begin(), M_DEVICE_EXTENSIONS.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();

}

QueueFamilyIndices MyVulkanDevice::GetQueueFamilyIndices(const VkPhysicalDevice& physicalDevice)
{
    QueueFamilyIndices indices;
    std::vector<VkQueueFamilyProperties> queueFamilies = GetQueueFamilies(physicalDevice);
    
    int i = 0;
    VkBool32 isPresentSupported = false;
    for (const auto& queueFamily : queueFamilies)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, mWindow.GetSurface(), &isPresentSupported);
        
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        if (isPresentSupported)
        {
            indices.presentFamily = i;
        }
        if (indices.IsComplete())
        {
            break;
        }
        i++;
    }

    return indices;
}

std::vector<VkQueueFamilyProperties> MyVulkanDevice::GetQueueFamilies(const VkPhysicalDevice& physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    return queueFamilies;
}

VkResult MyVulkanDevice::InitLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { mFamilyIndices.graphicsFamily.value(), mFamilyIndices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (const auto& queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};   
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    
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

    createInfo.enabledExtensionCount = static_cast<uint32_t>(M_DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = M_DEVICE_EXTENSIONS.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    return vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice);
}

} // namespace tlr
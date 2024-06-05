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

VkPhysicalDevice MyVulkanDevice::GetPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(mInstance.Get(), &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(mInstance.Get(), &physicalDeviceCount, physicalDevices.data());
    for (const auto& device : physicalDevices)
    {
        if (IsPhysicalDeviceSuitable(device))
        {
            return device;
        }
    }

    return VK_NULL_HANDLE;
}

bool MyVulkanDevice::IsPhysicalDeviceSuitable(const VkPhysicalDevice& physicalDevice)
{
    QueueFamilyIndices indices = GetQueueFamilyIndices(physicalDevice);
    return indices.IsComplete();
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

    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    createInfo.pEnabledFeatures = &deviceFeatures;

    return vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mLogicalDevice);
}


} // namespace tlr
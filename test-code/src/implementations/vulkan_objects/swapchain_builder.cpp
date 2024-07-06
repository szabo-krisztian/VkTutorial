#include "swapchain_builder.hpp"

namespace tlr
{

SwapchainBuilder::SwapchainBuilder()
{
    std::cout << "SDFA" << std::endl;
}

SwapchainBuilder::SwapchainBuilder(GLFWwindow* window, const VkSurfaceKHR& surface, const PhysicalDevice& physicalDevice, const Device& device)
{
    _info.window = window;
    _info.surface = surface;
    _info.physicalDevice = physicalDevice;
    _info.device = device;
    _info.graphicsQueueIndex = physicalDevice.familyIndices.graphicsFamily.value();
    _info.presentQueueIndex = physicalDevice.familyIndices.presentFamily.value();
}

Swapchain SwapchainBuilder::Build()
{
    SwapchainSupportDetails swapchainSupport = _info.physicalDevice.swapchainSupportDetails;
    VkSurfaceFormatKHR surfaceFormat = GetSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = GetSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = GetSwapExtent(swapchainSupport.capabilities);

    if (swapchainSupport.capabilities.maxImageCount > 0 && _info.imageCount > swapchainSupport.capabilities.maxImageCount)
    {
        _info.imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _info.surface;
    createInfo.minImageCount = _info.imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = _info.arrayLayerCount;
    createInfo.imageUsage = _info.imageUsageFlags;

    uint32_t queueFamilyIndices[] = {_info.graphicsQueueIndex, _info.presentQueueIndex};
    if (_info.graphicsQueueIndex != _info.presentQueueIndex)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    Swapchain swapchainInfo;
    VK_CHECK_RESULT(vkCreateSwapchainKHR(_info.device, &createInfo, nullptr, &swapchainInfo.swapchain), "swapchain creation failure!");
    
    return swapchainInfo;
}

SwapchainBuilder& SwapchainBuilder::SetDesiredColorSpace(VkColorSpaceKHR colorSpace)
{
    _info.desiredColorSpace = colorSpace;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::SetDesiredExtent(uint32_t width, uint32_t height)
{
    _info.desiredWidth = width;
    _info.desiredHeight = height;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::SetDesiredFormat(VkFormat format)
{
    _info.desiredFormat = format;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::SetDesiredPresentMode(VkPresentModeKHR presentMode)
{
    _info.desiredPresentMode = presentMode;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::SetImageFlags(VkImageUsageFlags usageFlags)
{
    _info.imageUsageFlags = usageFlags;
    return *this;
}

SwapchainBuilder& SwapchainBuilder::SetImageLayerCount(uint32_t arrayLayerCount)
{
    _info.arrayLayerCount = arrayLayerCount;
    return *this;
}

VkSurfaceFormatKHR SwapchainBuilder::GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        bool isFormatEnough = availableFormat.format == _info.desiredFormat && availableFormat.colorSpace == _info.desiredColorSpace;
        if (isFormatEnough)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapchainBuilder::GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == _info.desiredPresentMode) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapchainBuilder::GetSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width;
        int height;
        
        glfwGetFramebufferSize(_info.window, &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

} // namespace tlr
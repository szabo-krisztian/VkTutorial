#include "swapchain_builder.hpp"

namespace tlr
{

SwapchainBuilder::SwapchainBuilder(const PhysicalDevice& physicalDevice, const Device& device, const VkSurfaceKHR& surface)
{
    _info.physicalDevice = physicalDevice;
    _info.device = device;
    _info.surface = surface;
    _info.graphicsQueueIndex = physicalDevice.familyIndices.graphicsFamily.value();
    _info.presentQueueIndex = physicalDevice.familyIndices.presentFamily.value();
}

Swapchain SwapchainBuilder::Build()
{
    VkExtent2D extent =
    {
        static_cast<uint32_t>(_info.desiredWidth),
        static_cast<uint32_t>(_info.desiredHeight)
    };

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _info.surface;
    createInfo.minImageCount = _info.minImageCount;
    createInfo.imageFormat = _info.desiredFormat;
    createInfo.imageColorSpace = _info.desiredColorSpace;
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
    _info.presentMode = presentMode;
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

} // namespace tlr
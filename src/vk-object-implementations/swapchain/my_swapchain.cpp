#include "my_Swapchain.hpp"

namespace tlr
{

MySwapchain::MySwapchain(MyWindow& window, IDevice& device) : mWindow(window), mDevice(device)
{
    if (InitSwapchain() != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }
    InitSwapchainImages();
    InitSwapchainImageViews();
}

MySwapchain::~MySwapchain()
{
    for (auto imageView : mSwapchainImageViews)
    {
        vkDestroyImageView(mDevice.GetLogical(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(mDevice.GetLogical(), mSwapchain, nullptr);
}

VkSwapchainKHR MySwapchain::Get()
{
    return mSwapchain;
}

const VkFormat& MySwapchain::GetFormat() const
{
    return mSwapchainImageFormat;
}

const VkExtent2D& MySwapchain::GetExtent() const
{
    return mSwapchainExtent;
}

const std::vector<VkImageView>& MySwapchain::GetImageViews() const
{
    return mSwapchainImageViews;
}

VkResult MySwapchain::InitSwapchain()
{
    SwapchainSupportDetails SwapchainSupport = mDevice.GetSwapchainSupport();

    VkSurfaceFormatKHR surfaceFormat = GetSwapSurfaceFormat(SwapchainSupport.formats);
    VkPresentModeKHR presentMode = GetSwapPresentMode(SwapchainSupport.presentModes);
    VkExtent2D extent = GetSwapExtend(SwapchainSupport.capabilities);
    
    uint32_t imageCount = SwapchainSupport.capabilities.minImageCount + 1;
    if (SwapchainSupport.capabilities.maxImageCount > 0 && imageCount > SwapchainSupport.capabilities.maxImageCount)
    {
        imageCount = SwapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = mWindow.GetSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = mDevice.GetQueueFamilyIndices();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
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
    createInfo.preTransform = SwapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    mSwapchainImageFormat = surfaceFormat.format;
    mSwapchainExtent = extent;
    return vkCreateSwapchainKHR(mDevice.GetLogical(), &createInfo, nullptr, &mSwapchain);
}

VkSurfaceFormatKHR MySwapchain::GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        bool isFormatEnough = availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        if (isFormatEnough)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR MySwapchain::GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D MySwapchain::GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width;
        int height;
        glfwGetFramebufferSize(mWindow.GetWindow(), &width, &height);

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

void MySwapchain::InitSwapchainImages()
{
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(mDevice.GetLogical(), mSwapchain, &imageCount, nullptr);
    if (imageCount == 0)
    {
        throw std::runtime_error("zero swap chain images count!");
    }
    mSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice.GetLogical(), mSwapchain, &imageCount, mSwapchainImages.data());
}

void MySwapchain::InitSwapchainImageViews()
{
    mSwapchainImageViews.resize(mSwapchainImages.size());
    for (int i = 0; i < mSwapchainImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = mSwapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = mSwapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(mDevice.GetLogical(), &createInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

} // namespace tlr
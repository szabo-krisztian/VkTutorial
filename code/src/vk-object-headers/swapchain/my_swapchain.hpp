#pragma once

#include "my_window.hpp"
#include "idevice.h"

#include <cstdint>
#include <limits>
#include <algorithm>

namespace tlr
{

class MySwapchain
{
public:
    MySwapchain(MyWindow& window, IDevice& device);
    ~MySwapchain();

    MySwapchain(const MySwapchain&) = delete;
    MySwapchain operator=(const MySwapchain&) = delete;

    VkSwapchainKHR                  Get();
    const VkFormat&                 GetFormat()     const;
    const VkExtent2D&               GetExtent()     const;
    const std::vector<VkImageView>& GetImageViews() const;

private:
    MyWindow&                       mWindow;
    IDevice&                        mDevice;
    VkSwapchainKHR                  mSwapchain;
    std::vector<VkImage>            mSwapchainImages;
    VkFormat                        mSwapchainImageFormat;
    VkExtent2D                      mSwapchainExtent;
    std::vector<VkImageView>        mSwapchainImageViews;

    VkResult                        InitSwapchain();
    VkSurfaceFormatKHR              GetSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR                GetSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D                      GetSwapExtend(const VkSurfaceCapabilitiesKHR& capabilities);
    void                            InitSwapchainImages();
    void                            InitSwapchainImageViews();
};

} // namespace tlr
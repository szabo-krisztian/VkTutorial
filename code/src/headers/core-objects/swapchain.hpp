#pragma once

#include "state_board.hpp"
#include "iswapchain.h"

#include <cstdint>
#include <stdexcept>
#include <limits>
#include <algorithm>

namespace tlr
{

class Swapchain : public ISwapchain
{
public:
    Swapchain();
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain operator=(const Swapchain&) = delete;

    const VkSwapchainKHR&             Get()             const override;
    const VkFormat&                   GetFormat()       const override;
    const VkExtent2D&                 GetExtent()       const override;
    const std::vector<VkImageView>&   GetImageViews()   const override;

private:
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
    void                            InitSwapchainFramebuffers();
};

} // namespace tlr
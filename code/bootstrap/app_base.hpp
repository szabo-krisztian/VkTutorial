/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "physical_device.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "input_manager.hpp"
#include "camera.hpp"
#include "timer.hpp"
#include "deletion_queue.hpp"

namespace tlr
{

class AppBase
{
public:
    AppBase();
    ~AppBase();

    void Run();
    void ExitApp();

protected:
    const int WINDOW_WIDTH = 1920;
    const int WINDOW_HEIGHT = 1080;

    VkInstance                 instance;
    VkDebugUtilsMessengerEXT   debugMessenger;
    GLFWwindow                 *window;
    VkSurfaceKHR               surface;
    PhysicalDevice             physicalDevice;
    Device                     device;
    Swapchain                  swapchain;
    VkRenderPass               renderPass;
    std::vector<VkFramebuffer> framebuffers;
    InputManager               *inputManager;
    Camera                     camera;
    Timer                      timer;
    bool                       isAppRunning = false;

    virtual void Update() = 0;

private:
    struct DepthBuffer
    {
        VkImage        depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView    depthImageView;
    } _depthBuffer;

    DeletionQueue _deletionQueue;

    void Init();
    void InitGLFW();
    void InitVulkan();
    void InitSwapchain();

    void        InitDepthBuffer();
    VkFormat    FindDepthFormat();
    VkFormat    FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void        CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    void InitRenderPass();
    void InitFramebuffers();
    void InitInputManager();
};

} // namespace tlr

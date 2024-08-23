/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#include "app_base.hpp"

#include <array>

#include "toolset.hpp"
#include "initializers.hpp"
#include "instance_builder.hpp"
#include "physical_device_selector.hpp"
#include "device_builder.hpp"
#include "swapchain_builder.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (_deletionQueue).PushFunction(lambda)

namespace tlr
{

CameraCreateInfo cameraCI =
{
    glm::vec3(0.0f, 0.0f, 0.0f),    // initialPosition
    glm::vec3(0.0f, 1.0f, 0.0f),    // worldUp
    glm::radians(45.0f),            // fov
    1920.0f / 1080.0f,              // aspect
    1.57f,                          // initialYaw
    1.57f,                          // initialPitch
    0.001f,                         // sensitivity
    14.5f,                          // movementSpeed
    0.1f,                           // near
    200.0f                          // far
};

AppBase::AppBase() : camera(cameraCI)
{
    Init();
}

AppBase::~AppBase()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
}

void AppBase::Run()
{
    isAppRunning = true;

    while (!glfwWindowShouldClose(window) && isAppRunning)
    {
        glfwPollEvents();
        inputManager->Update();
        timer.Update();
        
        Update();
    }
}

void AppBase::ExitApp()
{
    isAppRunning = false;
}

void AppBase::Init()
{
    InitGLFW();
    InitVulkan();
    InitSwapchain();
    InitInputManager();
    InitDepthBuffer();
    InitRenderPass();
    InitFramebuffers();
}

void AppBase::InitGLFW()
{
    glfwInit();
    ENQUEUE_OBJ_DEL(( []() { glfwTerminate(); } ));

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
    ENQUEUE_OBJ_DEL(( [this]() { glfwDestroyWindow(window); } ));
}

void AppBase::InitVulkan()
{
    InstanceBuilder builder;
    Instance instances = builder.SetApplicationName("Vulkan app")
                                .SetApplicationVersion(VK_MAKE_VERSION(1, 0,0 ))
                                .SetEngineName("No engine")
                                .SetEngineVersion(VK_MAKE_VERSION(1, 0, 0))
                                .SetApiVersion(VK_MAKE_VERSION(1, 1, 0))
                                .RequestDefaultLayers()
                                .RequestDefaultExtensions()
                                .UseDebugMessenger()
                                .Build();
    instance = instances.instance;
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyInstance(instance, nullptr); } ));

    debugMessenger = instances.debugMessenger;
    ENQUEUE_OBJ_DEL(( [this]() { DestroyDebugMessenger(instance, debugMessenger); } ));

    VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroySurfaceKHR(instance, surface, nullptr); } ));

    PhysicalDeviceSelector physicalDeviceSelector{instance, surface};
    physicalDevice = physicalDeviceSelector.EnableDedicatedGPU()
                                           .Select();
    
    DeviceBuilder deviceBuilder{physicalDevice};
    device = deviceBuilder.EnableValidationLayers()
                          .Build();
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDevice(device, nullptr); } ));
}

void AppBase::InitSwapchain()
{
    SwapchainBuilder builder(window, surface, physicalDevice, device);
    swapchain = builder.SetDesiredFormat(VK_FORMAT_B8G8R8A8_SRGB)
                       .SetDesiredColorSpace(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                       .SetDesiredPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
                       .SetDesiredExtent(WINDOW_WIDTH, WINDOW_HEIGHT)
                       .SetDesiredImageCount(physicalDevice.swapchainSupportDetails.capabilities.minImageCount + 1)
                       .SetDesiredArrayLayerCount(1)
                       .SetImageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                       .Build();
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroySwapchainKHR(device, swapchain, nullptr); } ));
    for (auto& imageView : swapchain.imageViews)
    {
        ENQUEUE_OBJ_DEL(( [&]() { vkDestroyImageView(device, imageView, nullptr); } ));
    }
}

void AppBase::InitDepthBuffer()
{
    VkFormat depthFormat = FindDepthFormat();
    CreateImage(swapchain.extent.width, swapchain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthBuffer.depthImage, _depthBuffer.depthImageMemory);
    _depthBuffer.depthImageView = CreateImageView(_depthBuffer.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    ENQUEUE_OBJ_DEL(( [this]() {
        vkDestroyImage(device, _depthBuffer.depthImage, nullptr);
        vkFreeMemory(device, _depthBuffer.depthImageMemory, nullptr);
        vkDestroyImageView(device, _depthBuffer.depthImageView, nullptr);
    } ));
}

VkFormat AppBase::FindDepthFormat()
{
    return FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat AppBase::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    
    throw std::runtime_error("failed to find supported format!");
}

void AppBase::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo = init::ImageCreateInfo();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = init::MemoryAllocateInfo();
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.GetMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory));

    vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView AppBase::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = init::ImageViewCreateInfo();
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, &imageView));

    return imageView;
}

void AppBase::InitRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain.imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyRenderPass(device, renderPass, nullptr); } ));
}

void AppBase::InitFramebuffers()
{
    framebuffers.resize(swapchain.imageCount);
    for (int i = 0; i < framebuffers.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = {
            swapchain.imageViews[i],
            _depthBuffer.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = init::FramebufferCreateInfo();
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchain.extent.width;
        framebufferInfo.height = swapchain.extent.height;
        framebufferInfo.height = swapchain.extent.height;
        framebufferInfo.layers = 1;
        
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]));
        ENQUEUE_OBJ_DEL(( [this, i]() { vkDestroyFramebuffer(device, framebuffers[i], nullptr); } ));
    }
}

void AppBase::InitInputManager()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    InputManager::Init(window);
    inputManager = InputManager::GetInstance();
    
    inputManager->AddKeyPressListener(GLFW_KEY_ESCAPE, [&]() { ExitApp(); });
    
    inputManager->AddCursorPositionListener([&](float xoffset, float yoffset) { camera.CursorMovementCallback(xoffset, yoffset); });
    
    inputManager->AddKeyHoldListener(GLFW_KEY_W,          [&]() { camera.MoveForward(timer.GetDeltaTime());  });
    inputManager->AddKeyHoldListener(GLFW_KEY_A,          [&]() { camera.MoveLeft(timer.GetDeltaTime());     });
    inputManager->AddKeyHoldListener(GLFW_KEY_S,          [&]() { camera.MoveBackward(timer.GetDeltaTime()); });
    inputManager->AddKeyHoldListener(GLFW_KEY_D,          [&]() { camera.MoveRight(timer.GetDeltaTime());    });
    inputManager->AddKeyHoldListener(GLFW_KEY_SPACE,      [&]() { camera.MoveUp(timer.GetDeltaTime());       });
    inputManager->AddKeyHoldListener(GLFW_KEY_LEFT_SHIFT, [&]() { camera.MoveDown(timer.GetDeltaTime());     });
}

} // namespace tlr
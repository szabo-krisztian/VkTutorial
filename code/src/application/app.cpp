#include "app.hpp"
#include "toolset.hpp"
#include "initializers.hpp"

namespace tlr
{

App::App()
{
    InitQueues();
    InitCommands();
    InitSyncStructures();
}

App::~App()
{
    deleteQueue.flush();
}

App::FrameData& App::GetCurrentFrameData()
{
    return _frames[_frameNumber % FRAME_OVERLAP];
}

void App::InitQueues()
{
    _queues.graphicsQueueFamily = physicalDevice.familyIndices.graphicsFamily.value();
    vkGetDeviceQueue(device, _queues.graphicsQueueFamily, 0, &_queues.graphicsQueue);
    _queues.presentationQueueFamily = physicalDevice.familyIndices.presentFamily.value();
    vkGetDeviceQueue(device, _queues.presentationQueueFamily, 0, &_queues.presentationQueue);
}

void App::InitCommands()
{
    VkCommandPoolCreateInfo commandPoolCI = init::CommandPoolCreateInfo(_queues.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCI, nullptr, &_frames[i].commandPool), "failed to create command pool!");
        
        deleteQueue.push_function([this, i]() {
            vkDestroyCommandPool(device, _frames[i].commandPool, nullptr);
        });

        VkCommandBufferAllocateInfo commandBufferAI = init::CommandBufferAllocateInfo(_frames[i].commandPool, 1);
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAI, &_frames[i].mainCommandBuffer), "failed to allocate command buffer!");
    }
}

void App::InitSyncStructures()
{
    VkFenceCreateInfo fenceCI = init::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCI = init::SemaphoreCreateInfo();

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCI, nullptr, &_frames[i].renderFence), "failed to create fence!");
        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &_frames[i].swapchainSemaphore), "failed to create swapchain semaphore!");
        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &_frames[i].renderSemaphore), "failed to create render semaphore!");

        deleteQueue.push_function([this, i]() {
            vkDestroySemaphore(device, _frames[i].renderSemaphore, nullptr);
            vkDestroySemaphore(device, _frames[i].swapchainSemaphore, nullptr);
            vkDestroyFence(device, _frames[i].renderFence, nullptr);
        });
    }
}

void App::Draw()
{
    auto currentFrame = GetCurrentFrameData();
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &currentFrame.renderFence, true, 1000000000), "fence timeout!");
    VK_CHECK_RESULT(vkResetFences(device, 1, &currentFrame.renderFence), "failed to reset fence!");
    uint32_t swapchainImageIndex;
    VK_CHECK_RESULT(vkAcquireNextImageKHR(device, swapchain, 1000000000, currentFrame.swapchainSemaphore, nullptr, &swapchainImageIndex), "failed to acquire image from swapchain!");
    
    VkCommandBuffer cmd = currentFrame.mainCommandBuffer;
    VK_CHECK_RESULT(vkResetCommandBuffer(cmd, 0), "failed to reset command buffer");
    VkCommandBufferBeginInfo cmdBeginInfo = init::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &cmdBeginInfo), "failed to begin command buffer");
}

} // namespace tlr
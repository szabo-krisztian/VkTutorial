#include "app.hpp"
#include "toolset.hpp"
#include "initializers.hpp"

namespace tlr
{

App::App()
{
    InitQueues();
    InitCommands();
}

App::~App()
{
    DestroyObjects();
}

void App::DestroyObjects()
{
    vkDeviceWaitIdle(device);
    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        vkDestroyCommandPool(device, _frames[i].commandPool, nullptr);
    }
}

void App::InitQueues()
{
    _queues.graphicsQueueFamily = physicalDevice.familyIndices.graphicsFamily.value();
    vkGetDeviceQueue(device, _queues.graphicsQueueFamily, 0, &_queues.graphicsQueue);
    _queues.presentationQueueFamily = physicalDevice.familyIndices.presentFamily.value();
    vkGetDeviceQueue(device, _queues.presentationQueueFamily, 0, &_queues.presentationQueue);
}

App::FrameData& App::GetCurrentFrameData()
{
    return _frames[_frameNumber % FRAME_OVERLAP];
}

void App::InitCommands()
{
    VkCommandPoolCreateInfo commandPoolCI = init::CommandPoolCreateInfo(_queues.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCI, nullptr, &_frames[i].commandPool), "failed to create command pool!");

        VkCommandBufferAllocateInfo commandBufferAI = init::CommandBufferAllocateInfo(_frames[i].commandPool, 1);
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAI, &_frames[i].mainCommandBuffer), "failed to allocate command buffer!");
    }
}

} // namespace tlr
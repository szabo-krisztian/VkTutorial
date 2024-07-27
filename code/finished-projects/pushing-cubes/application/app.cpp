#include "app.hpp"

#include "initializers.hpp"
#include "toolset.hpp"

namespace tlr
{

App::App() 
{
    
    
    InitCommands();
    InitSyncStructures();
}

App::~App()
{
    
}

void App::Update()
{
       
}

void App::InitCommands()
{
    VkCommandPoolCreateInfo graphicsPoolCI = init::CommandPoolCreateInfo(device.queues.graphicsFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for (auto& frameData : _frames)
    {
        VK_CHECK_RESULT(vkCreateCommandPool(device, &graphicsPoolCI, nullptr, &frameData.commandPool));
        ENQUEUE_OBJ_DEL(( [&]() { vkDestroyCommandPool(device, frameData.commandPool, nullptr); } ));
        
        VkCommandBufferAllocateInfo commandBufferAI = init::CommandBufferAllocateInfo(frameData.commandPool, 1);
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAI, &frameData.commandBuffer));
    }

    VkCommandPoolCreateInfo transferPoolCI = init::CommandPoolCreateInfo(device.queues.graphicsFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    VK_CHECK_RESULT(vkCreateCommandPool(device, &transferPoolCI, nullptr, &_transferPool));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyCommandPool(device, _transferPool, nullptr); } ));
}

void App::InitSyncStructures()
{
    VkFenceCreateInfo fenceCI = init::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCI = init::SemaphoreCreateInfo();
    for (auto& frameData : _frames)
    {
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCI, nullptr, &frameData.renderFence));
        ENQUEUE_OBJ_DEL(( [&]() { vkDestroyFence(device, frameData.renderFence, nullptr); } ));

        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &frameData.swapchainSemaphore));
        ENQUEUE_OBJ_DEL(( [&]() { vkDestroySemaphore(device, frameData.swapchainSemaphore, nullptr); } ));

        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &frameData.renderSemaphore));
        ENQUEUE_OBJ_DEL(( [&]() { vkDestroySemaphore(device, frameData.renderSemaphore, nullptr); } ));
    }
}

App::FrameData& App::GetCurrentFrameData()
{
    return _frames[0];
}

} // namespace tlr

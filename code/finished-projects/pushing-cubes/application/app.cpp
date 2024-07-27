#include "app.hpp"

#include "initializers.hpp"
#include "toolset.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (_deletionQueue).PushFunction(lambda)

namespace tlr
{

CameraCreateInfo cameraCI = {
    glm::vec3(0.0f, 0.0f, 0.0f),   // initialPosition
    glm::vec3(0.0f, -1.0f, 0.0f),  // worldUp
    glm::radians(45.0f),           // fov
    800.0f / 600.0f,               // aspect
    90.0f,                         // initialYaw
    90.0f,                         // initialPitch
    0.1f,                          // sensitivity
    2.5f,                          // movementSpeed
    0.1f,                          // near
    100.0f                         // far
};

App::App() : _camera(cameraCI)
{
    inputManager->AddCursorPositionListener(std::bind(&Camera::CursorMovementCallback, _camera, std::placeholders::_1, std::placeholders::_2));
    
    InitCommands();
    InitSyncStructures();
}

App::~App()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
}

void App::Update()
{
    // Update logic
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

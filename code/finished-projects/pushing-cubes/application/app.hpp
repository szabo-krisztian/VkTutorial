#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#include "camera.hpp"

#define FRAME_OVERLAP 2

namespace tlr
{

class App : public AppBase
{
public:
    App();
    ~App();

protected:
    void Update() override;

private:
    struct FrameData
    {
        VkCommandPool   commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore     swapchainSemaphore, renderSemaphore;
        VkFence         renderFence;
    } _frames[FRAME_OVERLAP];
    
    int           _frameNumber = 0;
    VkCommandPool _transferPool;
    
    DeletionQueue _deletionQueue;
    Camera _camera;

    void InitCommands();
    void InitSyncStructures();
    FrameData& GetCurrentFrameData();
};

} // namespace tlr
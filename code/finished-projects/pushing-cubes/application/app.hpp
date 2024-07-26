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
    };
    int           _frameNumber = 0;
    FrameData     _frames[FRAME_OVERLAP];
    VkCommandPool _transferPool;
    
    DeletionQueue _deletionQueue;
    Camera _camera;
};

} // namespace tlr
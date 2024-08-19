#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "app_base.hpp"

#define FRAME_OVERLAP 2

namespace tlr
{

struct ModelTransform
{
    // alignas(16) glm::mat4 model;
    // alignas(16) glm::mat4 view;
    // alignas(16) glm::mat4 proj;
};

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
    }                _frames[FRAME_OVERLAP];
    int              _frameNumber = 0;
    VkCommandPool    _transferPool;
    
    DeletionQueue    _deletionQueue;

    void       InitCommands();
    void       InitSyncStructures();
    FrameData& GetCurrentFrameData();
};

} // namespace tlr
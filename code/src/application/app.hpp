#pragma once

#include <iostream>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#define FRAME_OVERLAP 2

namespace tlr
{

class App : public AppBase
{
public:
    App();
    ~App();

private:
    DeletionQueue deleteQueue;

    int _frameNumber = 0;
    struct FrameData
    {
        VkCommandPool   commandPool;
        VkCommandBuffer mainCommandBuffer;
        VkSemaphore     swapchainSemaphore, renderSemaphore;
        VkFence         renderFence;
    };
    FrameData _frames[FRAME_OVERLAP];

    struct Queues
    {
        uint32_t graphicsQueueFamily;
        VkQueue  graphicsQueue;
        uint32_t presentationQueueFamily;
        VkQueue  presentationQueue;
    } _queues;

    VkPipelineLayout _pipelineLayout;
    VkRenderPass     _renderPass;
    VkPipeline       _graphicsPipeline;

    FrameData& GetCurrentFrameData();
    void       InitQueues();
    void       InitCommands();
    void       InitSyncStructures();
    void       CreateGraphicsPipeline();
    void       CreateRenderPass();
};

} // namespace tlr
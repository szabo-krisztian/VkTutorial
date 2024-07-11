#pragma once

#include <iostream>

#include "app_base.hpp"
#define FRAME_OVERLAP 2

namespace tlr
{

class App : public AppBase
{
public:
    App();
    ~App();

private:
    int _frameNumber = 0;
    struct FrameData
    {
        VkCommandPool   commandPool;
        VkCommandBuffer mainCommandBuffer;
    };
    FrameData _frames[FRAME_OVERLAP];
    FrameData& GetCurrentFrameData();

    struct Queues
    {
        uint32_t graphicsQueueFamily;
        VkQueue  graphicsQueue;
        uint32_t presentationQueueFamily;
        VkQueue  presentationQueue;
    } _queues;


    void InitQueues();
    void InitCommands();
    void DestroyObjects();
};

} // namespace tlr
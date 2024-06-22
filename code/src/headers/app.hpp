#pragma once

#include "state_board.hpp"
#include "default_pipeline.hpp"

namespace tlr
{

class App
{
public:
    App();
    ~App();

    void DrawFrame();
    
private:
    DefaultPipeline            mPipeline;
    std::vector<VkFramebuffer> mFramebuffers;
    VkCommandPool              mCommandPool;
    VkCommandBuffer            mCommandBuffer;

    VkSemaphore                mImageAvailableSemaphore;
    VkSemaphore                mRenderFinishedSemaphore;
    VkFence                    mInFlightFence;

    void InitFramebuffers();
    void InitCommandPool();
    void InitCommandBuffer();
    void InitSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};

} // namespace tlr
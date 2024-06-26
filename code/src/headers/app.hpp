#pragma once

#include <array>

#include "state_board.hpp"
#include "default_pipeline.hpp"
#include "toolset.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

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
    
    VkCommandPool                                     mCommandPool;
    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> mCommandBuffers;

    uint32_t mCurrentFrame = 0;
    struct
    {
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
        std::array<VkFence, MAX_FRAMES_IN_FLIGHT>     inFlightFences;
    } mSynchronizationPrimitites;

    void InitFramebuffers();
    void InitCommandPool();
    void InitCommandBuffers();
    void InitSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
};

} // namespace tlr
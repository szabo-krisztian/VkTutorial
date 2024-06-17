#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "debug_instance_builder.hpp"
#include "my_instance.hpp"
#include "my_debug_messenger.hpp"
#include "my_window.hpp"
#include "my_device.hpp"
#include "my_swapchain.hpp"
#include "my_default_pipeline_builder.hpp"
#include "my_pipeline.hpp"

namespace tlr
{

class App
{
public:
    App();
    ~App();
    
    App(const App&) = delete;
    App operator=(const App&) = delete;

    void Run();

private:
    DebugInstanceBuilder mInstanceBuilder;
    MyInstance mInstance{mInstanceBuilder};
    MyDebugMessenger mMessenger{mInstance};
    MyWindow mWindow{mInstance};
    MyDevice mDevice{mInstance, mWindow};
    MySwapchain mSwapchain{mWindow, mDevice};
    DefaultPipelineBuilder mPipelineBuilder{mDevice};
    MyPipeline mPipeline{mPipelineBuilder.Build("spvs/vert.spv", "spvs/frag.spv", mSwapchain), mDevice};

    VkSemaphore mImageAvailableSemaphore;
    VkSemaphore mRenderFinishedSemaphore;
    VkFence mInFlightFence;
    std::vector<VkFramebuffer> mSwapchainFramebuffers;
    VkCommandPool mCommandPool;
    VkCommandBuffer mCommandBuffer;

    void InitSyncObjects();
    void InitFrameBuffers();
    void InitCommandPool();
    void InitCommandBuffer();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void Draw();
};

} // namespace tlr
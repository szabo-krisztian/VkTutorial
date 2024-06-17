#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <chrono>
#include <ctime>
#include <cmath>

#include "vertex.hpp"
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

    const int  M_MAX_FRAMES_IN_FLIGHT = 2;
    std::vector<VkSemaphore> mImageAvailableSemaphores;
    std::vector<VkSemaphore> mRenderFinishedSemaphores;
    std::vector<VkFence> mInFlightFences;

    std::vector<VkFramebuffer> mSwapchainFramebuffers;
    VkCommandPool mCommandPool;
    std::vector<VkCommandBuffer> mCommandBuffers;
    uint32_t mCurrentFrame;

    std::vector<Vertex> mVertices =
    {
        { { -0.75f,  0.75f }, { 1.0f, 0.0f, 0.0f } },
        { { 0.0f,  -0.75f }, { 0.0f, 1.0f, 0.0f } },
        { { 0.75f,  0.75f }, { 0.0f, 0.0f, 1.0f } }
    };
    VkBuffer mVertexBuffer;
    VkDeviceMemory mVertexBufferMemory;

    void     InitSyncObjects();
    void     InitFrameBuffers();
    void     InitCommandPool();
    void     InitCommandBuffers();
    void     RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void     CreateVertexBuffer();
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void     Update();
    void     Draw();
    void     DrawTriangle(Vertex v1, Vertex v2, Vertex v3, int depth);
};

} // namespace tlr
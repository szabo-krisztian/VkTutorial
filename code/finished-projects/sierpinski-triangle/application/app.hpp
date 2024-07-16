#pragma once

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#include "shader_vertex.hpp"

#define FRAME_OVERLAP 2

namespace tlr
{

class App : public AppBase
{
public:
    App(int fractalDepth);
    ~App();
    
    void Run();

private:
    DeletionQueue _deleteQueue;

    int _frameNumber = 0;
    struct FrameData
    {
        VkCommandPool   commandPool;
        VkCommandBuffer mainCommandBuffer;
        VkSemaphore     swapchainSemaphore, renderSemaphore;
        VkFence         renderFence;
    };
    FrameData _frames[FRAME_OVERLAP];
    std::vector<VkFramebuffer> _framebuffers;

    struct Queues
    {
        uint32_t graphicsQueueFamily;
        VkQueue  graphicsQueue;
        uint32_t presentationQueueFamily;
        VkQueue  presentationQueue;
    } _queues;

    VkPipelineLayout _pipelineLayout;
    VkRenderPass     _renderPass;
    VkBuffer         _vertexBuffer;
    VkDeviceMemory   _vertexBufferMemory;
    VkPipeline       _graphicsPipeline;

    FrameData& GetCurrentFrameData();
    void       InitQueues();
    void       InitCommands();
    void       InitSyncStructures();
    void       CreateVertexBuffer();
    uint32_t   FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void       CreateGraphicsPipeline();
    void       CreateRenderPass();
    void       CreateFramebuffers();
    void       PopulateSierpinskiTriangles(Vertex v1, Vertex v2, Vertex v3, int depth);
    void       RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
    void       DrawFrame();

    std::vector<Vertex> vertices = {
        {{0.0f, -0.9f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.9f, 0.9f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.9f, 0.9f, 0.0f}, {0.0f, 0.0f, 1.0f}}
    };
};

} // namespace tlr
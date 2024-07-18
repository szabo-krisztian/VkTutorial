#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#include "shader_vertex.hpp"

#define FRAME_OVERLAP 2

namespace tlr
{

class App : public AppBase
{
public:
    App();
    ~App();

    void Run();

private:
    struct FrameData
    {
        VkCommandPool   commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore     swapchainSemaphore, renderSemaphore;
        VkFence         renderFence;
    };
    int        _frameNumber = 0;
    FrameData  _frames[FRAME_OVERLAP];
    VkCommandPool _transferPool;

    struct Queues
    {
        uint32_t graphicsQueueFamily;
        VkQueue  graphicsQueue;
        uint32_t presentationQueueFamily;
        VkQueue  presentationQueue;
    } _queues;

    std::vector<Vertex> _vertices;

    VkBuffer       _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;

    VkRenderPass               _renderPass;
    std::vector<VkFramebuffer> _framebuffers;
    VkPipelineLayout           _pipelineLayout;
    VkPipeline                 _graphicsPipeline;

    
    DeletionQueue _deletionQueue;

    void       InitQueues();
    void       InitCommands();
    void       InitSyncStructures();
    FrameData& GetCurrentFrameData();
    void       PopulateVertices();
    void       CreateVertexBuffer();
    void       CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void       CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t   FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void       CreateRenderPass();
    void       CreateFramebuffers();
    void       CreateGraphicsPipeline();
    void       RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
    void       DrawFrame();
};

} // namespace tlr
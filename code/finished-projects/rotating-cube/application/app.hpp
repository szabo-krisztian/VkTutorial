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

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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

    const std::vector<Vertex> _vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> _indices = {
        0, 1, 2, 2, 3, 0
    };

    VkBuffer       _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;
    VkBuffer       _indicesBuffer;
    VkDeviceMemory _indicesBufferMemory;

    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
    std::vector<void*> _uniformBuffersMapped;


    VkRenderPass               _renderPass;
    std::vector<VkFramebuffer> _framebuffers;
    VkDescriptorSetLayout      _descriptorSetLayout;
    VkPipelineLayout           _pipelineLayout;
    VkPipeline                 _graphicsPipeline;

    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet>  _descriptorSets;

    DeletionQueue _deletionQueue;

    void       InitQueues();
    void       InitCommands();
    void       InitSyncStructures();
    FrameData& GetCurrentFrameData();
    void       PopulateVertices();
    void       CreateVertexBuffer();
    void       CreateIndexBuffer();
    void       CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void       CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t   FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    
    
    void       CreateDescriptorSetLayout();
    void       CreateUniformBuffers();
    void       UpdateUniformBuffer(uint32_t currentImage);
    void       CreateDescriptorPool();
    void       CreateDescriptorSets();


    void       CreateRenderPass();
    void       CreateFramebuffers();
    void       CreateGraphicsPipeline();
    void       RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
    void       DrawFrame();
};

} // namespace tlr
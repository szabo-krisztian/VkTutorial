#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#include "shader_vertex.hpp"
#include "input_manager.hpp"

#define FRAME_OVERLAP 2

namespace tlr
{

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
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
        {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},

        {{-0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}}
    };

    const std::vector<uint16_t> _indices = {
        0, 1, 2, 2, 3, 0,

        7, 6, 5, 7, 5, 4,

        1, 6, 2, 1, 5, 6,

        4, 0, 3, 4, 3, 7,

        3, 2, 6, 3, 6, 7,

        4, 5, 1, 4, 1, 0
    };

    Buffer _vertexBuffer;
    Buffer _indexBuffer;

    std::vector<Buffer> _uniformBuffers;

    VkRenderPass               _renderPass;
    std::vector<VkFramebuffer> _framebuffers;
    VkDescriptorSetLayout      _descriptorSetLayout;
    VkPipelineLayout           _pipelineLayout;
    VkPipeline                 _graphicsPipeline;

    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet>  _descriptorSets;

    struct DepthBuffer
    {
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
    } _depthBuffer;
    
    InputManager* _inputManager;
    DeletionQueue _deletionQueue;

    void       InitCommands();
    void       InitSyncStructures();
    FrameData& GetCurrentFrameData();
    void       CreateVertexBuffer();
    void       CreateIndexBuffer();
    void       CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
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

    
    void       CreateDepthResources();
    VkFormat   FindDepthFormat();
    VkFormat   FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void       CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

} // namespace tlr
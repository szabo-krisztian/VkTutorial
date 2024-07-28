#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#include "shader_vertex.hpp"

#define FRAME_OVERLAP 2
#define BULLET_COUNT 10

namespace tlr
{

struct UniformBufferObject
{
    //alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class App : public AppBase
{
public:
    App();
    ~App();

protected:
    void Update() override;

private:
    struct FrameData
    {
        VkCommandPool   commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore     swapchainSemaphore, renderSemaphore;
        VkFence         renderFence;
    };
    int              _frameNumber = 0;
    FrameData        _frames[FRAME_OVERLAP];
    VkCommandPool    _transferPool;
    VkDescriptorPool _descriptorPool;

    Buffer                _cameraTransformBuffer[FRAME_OVERLAP];
    VkDescriptorSetLayout _cameraTransformLayout;
    VkDescriptorSet       _cameraTransformSets[FRAME_OVERLAP];

    const std::vector<Vertex> _boxVertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},

        {{-0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, -0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}}
    };

    const std::vector<uint16_t> _boxIndices = {
        0, 1, 2, 2, 3, 0,

        7, 6, 5, 7, 5, 4,

        1, 6, 2, 1, 5, 6,

        4, 0, 3, 4, 3, 7,

        3, 2, 6, 3, 6, 7,

        4, 5, 1, 4, 1, 0
    };

    Buffer _boxVertexBuffer;
    Buffer _boxIndexBuffer;

    Buffer                _uniformBuffers[FRAME_OVERLAP];
    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorSet       _descriptorSets[FRAME_OVERLAP];
    
    
    const std::vector<Vertex> _bulletVertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, 
        {{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},   
        {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},  
        {{0.0f, 0.0f, 0.5f}, {1.0f, 0.0f, 0.0f}}    
    };

    const std::vector<uint16_t> _bulletIndices = {
        0, 2, 3,
        2, 1, 3,
        1, 0, 3,
        1, 2, 0
    };

    Buffer _bulletVertexBuffer;
    Buffer _bulletIndexBuffer;

    

    VkRenderPass               _renderPass;
    std::vector<VkFramebuffer> _framebuffers;
    VkPipelineLayout           _pipelineLayout;
    VkPipeline                 _graphicsPipeline;

    struct DepthBuffer
    {
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
    } _depthBuffer;
    
    DeletionQueue _deletionQueue;

    void        InitCommands();
    void        InitSyncStructures();
    FrameData&  GetCurrentFrameData();
    
    void        CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void        CreateCubeVertexBuffer();
    void        CreateCubeIndexBuffer();
    void        CreateBulletVertexBuffer();
    void        CreateBulletIndexBuffer();    

    void        CreateCameraTransformDescriptorSetLayout();
    void        CreateCameraTransformBuffer();
    void        CreateCameraTransformDescriptorSets();


    void        CreateDescriptorSetLayout();
    void        CreateUniformBuffers();
    void        UpdateUniformBuffer(uint32_t currentImage);
    void        UpdateModel(uint32_t currentImage);
    void        CreateDescriptorPool();
    void        CreateDescriptorSets();

    void        CreateRenderPass();
    void        CreateFramebuffers();
    void        CreateGraphicsPipeline();
    void        RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
    
    void        CreateDepthResources();
    VkFormat    FindDepthFormat();
    VkFormat    FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void        CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

} // namespace tlr
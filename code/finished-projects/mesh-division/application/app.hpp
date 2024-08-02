#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#include "shader_vertex.hpp"

#define FRAME_OVERLAP 2

namespace tlr
{

struct CameraTransform
{
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
    DeletionQueue    _deletionQueue;

    struct
    {
        Buffer                ubos[FRAME_OVERLAP];
        VkDescriptorSetLayout layout;
        VkDescriptorSet       sets[FRAME_OVERLAP];
    } _cameraTransform;

    VkDescriptorSetLayout _modelTransformLayout;

    struct
    {
        Buffer              vertexBuffer;
        std::vector<Vertex> vertices;
        Buffer              transformBuffers[FRAME_OVERLAP];
        VkDescriptorSet     transformSets[FRAME_OVERLAP];
    } _mainMesh;

    struct DepthBuffer
    {
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;
    } _depthBuffer;

    VkRenderPass               _renderPass;
    std::vector<VkFramebuffer> _framebuffers;
    VkPipelineLayout           _pipelineLayout;
    VkPipeline                 _graphicsPipeline;


    void        InitCommands();
    void        InitSyncStructures();
    FrameData&  GetCurrentFrameData();
    
    void        CreateMainMeshVertices();
    void        CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void        CreateMainMeshVertexBuffer();
    
    void        CreateDescriptorPool();
    void        CreateCameraTransformDescriptorSetLayout();
    void        CreateCameraTransformUniformBuffers();
    void        CreateCameraTransformDescriptorSets();
    void        UpdateCameraTransform(uint32_t currentImage);
    void        CreateModelTransformDescriptorSetLayout();
    void        CreateMainMeshTransformUniformBuffers();
    void        CreateMainMeshTransformDescriptorSets();
    void        UpdateMainMeshTransform(uint32_t currentImage);

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
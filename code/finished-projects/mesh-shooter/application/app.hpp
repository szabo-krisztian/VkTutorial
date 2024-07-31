#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#include "shader_vertex.hpp"
#include "simulator.hpp"
#include "utilities.hpp"

#define FRAME_OVERLAP 2
#define BULLET_COUNT 10

namespace tlr
{

struct UniformBufferObject
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

    struct
    {
        Buffer                ubos[FRAME_OVERLAP];
        VkDescriptorSetLayout layout;
        VkDescriptorSet       sets[FRAME_OVERLAP];
    } _cameraTransform;

    VkDescriptorSetLayout _modelTransformLayout;

    Buffer _boxVertexBuffer;
    std::vector<Vertex> _boxVertices;

    struct
    {
        Buffer          ubos[FRAME_OVERLAP];
        VkDescriptorSet sets[FRAME_OVERLAP];
    } _cubeTransform;
    
    Buffer _bulletVertexBuffer;
    std::vector<Vertex> _bulletVertices;

    struct
    {
        Buffer          ubos[FRAME_OVERLAP * BULLET_COUNT];
        VkDescriptorSet sets[FRAME_OVERLAP * BULLET_COUNT];
        size_t          count = 0;  
    } _bulletTransforms;

    VkRenderPass               _renderPass;
    std::vector<VkFramebuffer> _framebuffers;
    VkPipelineLayout           _pipelineLayout;
    VkPipeline                 _graphicsPipeline;
    Simulator                  _simulator;

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
    void        CreateCubeVertices();
    void        CreateCubeVertexBuffer();
    void        CreateBulletVertices();
    void        CreateBulletVertexBuffer();



    void        CreateDescriptorPool();
    void        CreateCameraTransformDescriptorSetLayout();
    void        CreateCameraTransformUniformBuffers();
    void        CreateCameraTransformDescriptorSets();
    void        UpdateCameraTransform(uint32_t currentImage);
    void        CreateModelTransformDescriptorSetLayout();
    void        CreateCubeTransformUniformBuffers();
    void        CreateCubeTransformDescriptorSets();
    void        UpdateCubeTransform(uint32_t currentImage);
    void        CreateBulletTransformsUniformBuffers();
    void        CreateBulletTransformsDescriptorSets();
    void        UpdateBulletTransforms(uint32_t currentImage);



    void        CreateRenderPass();
    void        CreateFramebuffers();
    void        CreateGraphicsPipeline();
    void        RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
    void        ShootBullet();



    void        CreateDepthResources();
    VkFormat    FindDepthFormat();
    VkFormat    FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void        CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

} // namespace tlr
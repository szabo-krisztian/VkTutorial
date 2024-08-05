#pragma once

#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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
    }                _frames[FRAME_OVERLAP];;
    int              _frameNumber = 0;
    VkCommandPool    _transferPool;
    VkDescriptorPool _descriptorPool;

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
    
    Buffer              _bulletVertexBuffer;
    std::vector<Vertex> _bulletVertices;
    struct
    {
        Buffer          ubos[FRAME_OVERLAP * BULLET_COUNT];
        VkDescriptorSet sets[FRAME_OVERLAP * BULLET_COUNT];
        size_t          count = 0;  
    } _bulletTransforms;

    VkPipelineLayout           _pipelineLayout;
    VkPipeline                 _graphicsPipeline;
    Simulator                  _simulator;
    
    DeletionQueue _deletionQueue;
    

    void        InitCommands();
    void        InitSyncStructures();
    FrameData&  GetCurrentFrameData();

    void        CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void        CreateMainMeshVertices();
    void        CreateMainMeshVertexBuffer();
    void        CreateBulletVertices();
    void        CreateBulletVertexBuffer();

    void        CreateDescriptorPool();
    void        CreateCameraTransformDescriptorSetLayout();
    void        CreateCameraTransformUniformBuffers();
    void        CreateCameraTransformDescriptorSets();
    void        UpdateCameraTransform(uint32_t currentImage);
    void        CreateModelTransformDescriptorSetLayout();
    void        CreateMainMeshTransformUniformBuffers();
    void        CreateMainMeshTransformDescriptorSets();
    void        UpdateMainMeshTransform(uint32_t currentImage);
    void        CreateBulletTransformsUniformBuffers();
    void        CreateBulletTransformsDescriptorSets();
    void        UpdateBulletTransforms(uint32_t currentImage);

    void        CreateGraphicsPipeline();
    void        RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
    void        ShootBullet();
};

} // namespace tlr
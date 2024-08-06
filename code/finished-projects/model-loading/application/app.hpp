#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "app_base.hpp"
#include "vertex.hpp"

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
    const std::string MODEL_PATH = "C:/dev/VulkanProjs/Start/code/finished-projects/model-loading/application/bugatti/bugatti.obj";
    const std::string MTL_PATH = "C:/dev/VulkanProjs/Start/code/finished-projects/model-loading/application/bugatti";

    struct FrameData
    {
        VkCommandPool   commandPool;
        VkCommandBuffer commandBuffer;
        VkSemaphore     swapchainSemaphore, renderSemaphore;
        VkFence         renderFence;
    }                _frames[FRAME_OVERLAP];
    int              _frameNumber = 0;
    VkCommandPool    _transferPool;
    VkDescriptorPool _descriptorPool;

    struct Material
    {
        std::string name;
        float specularExponent;
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        glm::vec3 emissive;
        float opticalDensity;
        float alpha;
    };
    std::vector<Material> _materials;

    std::vector<Vertex> vertices;
    Buffer buffer;
    
    struct
    {
        VkDescriptorSetLayout layout;
        Buffer              transformBuffers[FRAME_OVERLAP];
        VkDescriptorSet     transformSets[FRAME_OVERLAP];
    } _model;


    void InitCommands();
    void InitSyncStructures();
    FrameData& GetCurrentFrameData();

    void ReadMeshInfo();
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void CreateMainMeshVertexBuffer();

    void        CreateDescriptorPool();
    void        CreateCameraTransformDescriptorSetLayout();
    void        CreateCameraTransformUniformBuffers();
    void        CreateCameraTransformDescriptorSets();
    void        UpdateCameraTransform(uint32_t currentImage);
    void        CreateModelTransformDescriptorSetLayout();
    void        CreateMainMeshTransformUniformBuffers();
    void        CreateMainMeshTransformDescriptorSets();
    void        UpdateMainMeshTransform(uint32_t currentImage);

     void        CreateGraphicsPipeline();
    void        RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);

    struct
    {
        Buffer                ubos[FRAME_OVERLAP];
        VkDescriptorSetLayout layout;
        VkDescriptorSet       sets[FRAME_OVERLAP];
    } _cameraTransform;

    VkPipelineLayout _pipelineLayout;
    VkPipeline       _graphicsPipeline;

    DeletionQueue _deletionQueue;

};

} // namespace tlr
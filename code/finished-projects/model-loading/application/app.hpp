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

// Camera Transform
struct alignas(16) CameraTransform
{
    glm::mat4 view;  // 64 bytes
    glm::mat4 proj;  // 64 bytes
};

// Ensure the size is a multiple of 16 bytes
static_assert(sizeof(CameraTransform) % 16 == 0, "CameraTransform struct size must be a multiple of 16 bytes");

// Model Transform
struct alignas(16) ModelTransform
{
    glm::mat4 vertex; // 64 bytes
    glm::mat3 normal; // 36 bytes
    
    // Padding to align to 16 bytes
    alignas(16) glm::vec3 padding; // 12 bytes padding (not strictly necessary but ensures alignment)

    // Ensure the size is a multiple of 16 bytes
    // Total: 64 (vertex) + 36 (normal) + 12 (padding) = 112 bytes -> padded to 128 bytes
};

// Ensure the size is a multiple of 16 bytes
static_assert(sizeof(ModelTransform) % 16 == 0, "ModelTransform struct size must be a multiple of 16 bytes");

// Material
struct alignas(16) Material
{
    float specularExponent;  // 4 bytes
    float padding1;          // 4 bytes (padding)
    glm::vec3 ambient;       // 12 bytes
    float padding2;          // 4 bytes (padding)
    glm::vec3 diffuse;       // 12 bytes
    float padding3;          // 4 bytes (padding)
    glm::vec3 specular;      // 12 bytes
    float padding4;          // 4 bytes (padding)
    glm::vec3 emissive;      // 12 bytes
    float alpha;             // 4 bytes

    // Ensure the size is a multiple of 16 bytes
    // Total: 4 (specularExponent) + 4 (padding1) + 12 (ambient) + 4 (padding2) + 12 (diffuse) + 4 (padding3) + 12 (specular) + 4 (padding4) + 12 (emissive) + 4 (alpha) = 76 bytes -> padded to 80 bytes
};

// Ensure the size is a multiple of 16 bytes
static_assert(sizeof(Material) % 16 == 0, "Material struct size must be a multiple of 16 bytes");

// Light
struct alignas(16) Light
{
    glm::vec3 position;  // 12 bytes
    float padding1;      // 4 bytes (padding)
    glm::vec3 lightColor; // 12 bytes
    float padding2;      // 4 bytes (padding)
    float lightPower;    // 4 bytes
    float padding3;      // 4 bytes (padding)

    // Ensure the size is a multiple of 16 bytes
    // Total: 12 (position) + 4 (padding1) + 12 (lightColor) + 4 (padding2) + 4 (lightPower) + 4 (padding3) = 40 bytes -> padded to 48 bytes
};

// Ensure the size is a multiple of 16 bytes
static_assert(sizeof(Light) % 16 == 0, "Light struct size must be a multiple of 16 bytes");


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

    
    VkDescriptorSetLayout layoutSet1;
    VkDescriptorSet sets1[FRAME_OVERLAP];

    VkDescriptorSetLayout layoutSet2;
    std::vector<VkDescriptorSet> sets2;

    std::vector<Material> _materials;
    std::unordered_map<int, std::vector<Vertex>> _shapeVertices;
    std::unordered_map<int, Buffer> _shapeBuffers;
    
    struct
    {
        std::vector<Buffer> buffers;
        std::vector<VkDescriptorSet> sets;
    } _mats;

    struct
    {
        Buffer          buffers[FRAME_OVERLAP];
    } _light;
    
    
    struct
    {
        Buffer          buffers[FRAME_OVERLAP];
    } _cam;

    struct
    {
        Buffer          buffers[FRAME_OVERLAP];
    } _model;

    struct
    {
        Buffer                ubos[FRAME_OVERLAP];
        VkDescriptorSetLayout layout;
        VkDescriptorSet       sets[FRAME_OVERLAP];
    } _cameraTransform;

    VkPipelineLayout _pipelineLayout;
    VkPipeline       _graphicsPipeline;

    DeletionQueue _deletionQueue;

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
    void        UpdateOtherSets(uint32_t currentImage);

     void        CreateGraphicsPipeline();
    void        RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);

    

};

} // namespace tlr
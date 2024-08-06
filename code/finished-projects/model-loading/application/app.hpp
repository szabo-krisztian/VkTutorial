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

struct ModelTransform
{
    alignas(16) glm::mat4 vertexTransform;
    alignas(16) glm::mat4 normalTransform;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct Material
{
    alignas(4)  float     specularExponent;
    alignas(16) glm::vec3 ambient;
    alignas(16) glm::vec3 diffuse;
    alignas(16) glm::vec3 specular;
    alignas(16) glm::vec3 emissive;
    alignas(4)  float     alpha;
};


struct Light
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 lightColor;
    alignas(4)  float     lightPower;
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

    void       InitCommands();
    void       InitSyncStructures();
    FrameData& GetCurrentFrameData();

    struct
    {
        std::vector<Material> materials;
        std::vector<Buffer>   buffers;
        std::unordered_map<int, std::vector<Vertex>> vertices;
    } _mesh;

    void ReadMeshInfo();
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void InitMeshVertexBuffer();

    VkDescriptorSetLayout _set0Layout;
    VkDescriptorSetLayout _set1Layout;
    VkDescriptorPool      _descriptorPool;

    void CreateDescriptorLayouts();
    void CreateDescriptorPool();

    struct Set0
    {
        Buffer modelUbo;
        Buffer lightUbo;
        Buffer cameraPositionUbo;
        VkDescriptorSet set;
    };
    std::vector<Set0> _sets0;

    struct Set1
    {
        Buffer materialUbo;
        VkDescriptorSet set;
    };
    std::vector<Set1> _sets1;

    void CreateDescriptorSets();
    void UpdateDesciptorUbos(uint32_t frameNumber);

    VkPipelineLayout _pipelineLayout;
    VkPipeline       _graphicsPipeline;
    DeletionQueue    _deletionQueue;

    void        CreateGraphicsPipeline();
    void        RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);
};

} // namespace tlr
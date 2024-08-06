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

    std::unordered_map<int, std::vector<Vertex>> _vertices;

    void InitCommands();
    void InitSyncStructures();
    FrameData& GetCurrentFrameData();

    void ReadMeshInfo();

    struct
    {
        Buffer                ubos[FRAME_OVERLAP];
        VkDescriptorSetLayout layout;
        VkDescriptorSet       sets[FRAME_OVERLAP];
    } _cameraTransform;

    DeletionQueue _deletionQueue;

};

} // namespace tlr
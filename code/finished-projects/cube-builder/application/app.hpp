#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "app_base.hpp"
#include "world.hpp"
#include "shader_vertex.hpp"
#include "cube_push_constant.hpp"

#define FRAME_OVERLAP 2

namespace tlr
{

struct CameraTransform
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

template <typename LayoutUBO>
struct Layout
{
    VkDescriptorSetLayout layout;
    operator VkDescriptorSetLayout&() { return layout; }
    operator VkDescriptorSetLayout*() { return &layout; }

    std::vector<LayoutUBO>       ubos;
    std::vector<VkDescriptorSet> sets;
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
    }                _frames[FRAME_OVERLAP];
    int              _frameNumber = 0;
    VkCommandPool    _transferPool;
    
    void       InitCommands();
    void       InitSyncStructures();
    FrameData& GetCurrentFrameData();

    struct
    {
        Buffer vertexBuffer;
        std::vector<VertexInfo> vertices;
    } _cube;
    
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void InitMeshVertexBuffer();


    struct Layout0UBO
    {
        Buffer cameraTransform;
    };
    VkDescriptorPool   _descriptorPool;
    Layout<Layout0UBO> _layout0;

    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();

    World _world;
    DeletionQueue _deletionQueue;

    
};

} // namespace tlr
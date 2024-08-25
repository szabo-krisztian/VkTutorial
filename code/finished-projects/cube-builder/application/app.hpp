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

    /*
    struct
    {
        Buffer vertexBuffer;
        std::vector<VertexInfo> vertices;
    } _cube;

    VkPipelineLayout _pipelineLayout;
    VkPipeline       _graphicsPipeline;
    */

    DeletionQueue    _deletionQueue;

    World _world;

   
};

} // namespace tlr
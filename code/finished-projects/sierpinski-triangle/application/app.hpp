#pragma once

#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "app_base.hpp"
#include "deletion_queue.hpp"
#include "shader_vertex.hpp"

#define FRAME_OVERLAP 2

namespace tlr
{

class App : public AppBase
{
public:
    App(int fractalDepth);
    ~App();

protected:
    void Update() override;

private:
    struct FrameData
    {
        VkCommandPool   commandPool;
        VkCommandBuffer mainCommandBuffer;
        VkSemaphore     swapchainSemaphore, renderSemaphore;
        VkFence         renderFence;
    };
    int       _frameNumber = 0;
    FrameData _frames[FRAME_OVERLAP];

    VkPipelineLayout _pipelineLayout;
    VkBuffer         _vertexBuffer;
    VkDeviceMemory   _vertexBufferMemory;
    VkPipeline       _graphicsPipeline;

    DeletionQueue _deleteQueue;

    FrameData& GetCurrentFrameData();
    void       InitCommands();
    void       InitSyncStructures();
    void       CreateVertexBuffer();
    uint32_t   FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void       CreateGraphicsPipeline();
    void       PopulateSierpinskiTriangles(Vertex v1, Vertex v2, Vertex v3, int depth);
    void       RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex);

    std::vector<Vertex> vertices = {
        {{0.0f, -0.9f, 0.9f}, {1.0f, 0.0f, 0.0f}},
        {{0.9f, 0.9f, 0.9f}, {0.0f, 1.0f, 0.0f}},
        {{-0.9f, 0.9f, 0.9f}, {0.0f, 0.0f, 1.0f}}
    };
};

} // namespace tlr
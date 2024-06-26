#pragma once

#include <array>

#include "state_board.hpp"
#include "default_pipeline.hpp"
#include "toolset.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

namespace tlr
{

class App
{
public:
    App();
    ~App();

    void DrawFrame();
    
private:
    DefaultPipeline            mPipeline;
    std::vector<VkFramebuffer> mFramebuffers;
    
    VkCommandPool                                     mCommandPool;
    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> mCommandBuffers;

    uint32_t mCurrentFrame = 0;
    struct
    {
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores;
        std::array<VkFence, MAX_FRAMES_IN_FLIGHT>     inFlightFences;
    } mSynchronizationPrimitites;

    struct {
		VkDeviceMemory memory{ VK_NULL_HANDLE };
		VkBuffer buffer;
	} mVertices;

    struct UniformBuffer {
		VkDeviceMemory memory;
		VkBuffer buffer;
		VkDescriptorSet descriptorSet;
		uint8_t* mapped{ nullptr };
	};
    std::array<UniformBuffer, MAX_FRAMES_IN_FLIGHT> mUniformBuffers;

    struct ShaderData {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
	};

    void InitFramebuffers();
    void InitCommandPool();
    void InitCommandBuffers();
    void InitSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkPhysicalDeviceMemoryProperties deviceMemoryProperties);
    void CreateVertexBuffer();
};

} // namespace tlr
#include "app.hpp"

#include "initializers.hpp"
#include "toolset.hpp"
#include "shader_module.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (_deletionQueue).PushFunction(lambda)

namespace tlr
{

std::string GetAbsolutePath(const std::string& relativePath)
{
    std::string fullPath(__FILE__);
    std::string directory = fullPath.substr(0, fullPath.find_last_of("\\/") + 1);
    std::string absolutePath = directory + relativePath;
    return absolutePath;
}

App::App()
{
    InitCommands();
    InitSyncStructures();

    CreateDescriptorSetLayout();
    CreateDescriptorPool();
}

App::~App()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
}

void App::Update()
{
    
}


void App::InitCommands()
{
    VkCommandPoolCreateInfo transferCommandPoolCI = init::CommandPoolCreateInfo(device.queues.graphicsFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    VK_CHECK_RESULT(vkCreateCommandPool(device, &transferCommandPoolCI, nullptr, &_transferPool));
    ENQUEUE_OBJ_DEL([this] { vkDestroyCommandPool(device, _transferPool, nullptr); });

    VkCommandPoolCreateInfo commandPoolCI = init::CommandPoolCreateInfo(device.queues.graphicsFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCI, nullptr, &_frames[i].commandPool));
        ENQUEUE_OBJ_DEL(( [this, i] { vkDestroyCommandPool(device, _frames[i].commandPool, nullptr); } ));

        VkCommandBufferAllocateInfo commandBufferAI = init::CommandBufferAllocateInfo(_frames[i].commandPool, 1);
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAI, &_frames[i].commandBuffer));
    }
}

void App::InitSyncStructures()
{
    VkFenceCreateInfo fenceCI = init::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo semaphoreCI = init::SemaphoreCreateInfo();

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK_RESULT(vkCreateFence(device, &fenceCI, nullptr, &_frames[i].renderFence));
        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &_frames[i].swapchainSemaphore));
        VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &_frames[i].renderSemaphore));

        ENQUEUE_OBJ_DEL(( [this, i] {
            vkDestroySemaphore(device, _frames[i].renderSemaphore, nullptr);
            vkDestroySemaphore(device, _frames[i].swapchainSemaphore, nullptr);
            vkDestroyFence(device, _frames[i].renderFence, nullptr);
        } ));
    }
}

App::FrameData& App::GetCurrentFrameData()
{
    return _frames[_frameNumber];
}

void App::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = init::CommandBufferAllocateInfo(_transferPool, 1);
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = init::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = init::SubmitInfo();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(device.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device.queues.graphics);
    
    vkFreeCommandBuffers(device, _transferPool, 1, &commandBuffer);
}

void App::InitMeshVertexBuffer()
{
    /*
        init _cube.vertices with actual raw data
     */

    VkDeviceSize bufferSize = sizeof(_cube.vertices[0]) * _cube.vertices.size();
    
    Buffer stagingBuffer;
    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, bufferSize));
    VK_CHECK_RESULT(stagingBuffer.Map());
    stagingBuffer.CopyTo(_cube.vertices.data(), bufferSize);

    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_cube.vertexBuffer, bufferSize));
    ENQUEUE_OBJ_DEL(([this] { _cube.vertexBuffer.Destroy(); }));

    CopyBuffer(stagingBuffer.buffer, _cube.vertexBuffer.buffer, bufferSize);

    stagingBuffer.Destroy();
}

void App::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding viewBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
    VkDescriptorSetLayoutBinding projBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1, 1);
    VkDescriptorSetLayoutBinding bindings[] =
    {
        viewBinding, projBinding
    };
    
    VkDescriptorSetLayoutCreateInfo layoutInfoSet0 = init::DescriptorSetLayoutCreateInfo(2, bindings);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfoSet0, nullptr, _layout0));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyDescriptorSetLayout(device, _layout0, nullptr); } ));
}

void App::CreateDescriptorPool()
{
    uint32_t viewTransformCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize viewTransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, viewTransformCount);

    uint32_t projTransformCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize projTransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, projTransformCount);

    uint32_t maxDescriptorCount = viewTransformCount + projTransformCount;
    VkDescriptorPoolSize poolsizes[] = {viewTransformSize, projTransformSize};
    VkDescriptorPoolCreateInfo poolInfo = init::DescriptorPoolCreateInfo(2, poolsizes, maxDescriptorCount);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyDescriptorPool(device, _descriptorPool, nullptr); } ));
}

} // namespace tlr
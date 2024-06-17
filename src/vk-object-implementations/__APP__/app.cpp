#include "app.hpp"

namespace tlr
{

App::App()
{
    glfwInit();
    InitSyncObjects();
    InitFrameBuffers();
    InitCommandPool();
    InitCommandBuffers();
    CreateVertexBuffer();
    mCurrentFrame = 0;
}

App::~App()
{
    for (int i = 0; i < M_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(mDevice.GetLogical(), mImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(mDevice.GetLogical(), mRenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(mDevice.GetLogical(), mInFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(mDevice.GetLogical(), mCommandPool, nullptr);

    for (auto framebuffer : mSwapchainFramebuffers)
    {
        vkDestroyFramebuffer(mDevice.GetLogical(), framebuffer, nullptr);
    }
    
    vkDestroyBuffer(mDevice.GetLogical(), mVertexBuffer, nullptr);
    vkFreeMemory(mDevice.GetLogical(), mVertexBufferMemory, nullptr);
    glfwTerminate();
}

void App::Run()
{
    while (mWindow.IsWindowActive())
    {
        glfwPollEvents();
        Update();
    }
    vkDeviceWaitIdle(mDevice.GetLogical());
}



void App::InitSyncObjects()
{
    mImageAvailableSemaphores.resize(M_MAX_FRAMES_IN_FLIGHT);
    mRenderFinishedSemaphores.resize(M_MAX_FRAMES_IN_FLIGHT);
    mInFlightFences.resize(M_MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < M_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(mDevice.GetLogical(), &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(mDevice.GetLogical(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(mDevice.GetLogical(), &fenceInfo, nullptr, &mInFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

void App::InitFrameBuffers()
{
    mSwapchainFramebuffers.resize(mSwapchain.GetImageViews().size());

    for (size_t i = 0; i < mSwapchain.GetImageViews().size(); i++)
    {
        VkImageView attachments[] =
        {
            mSwapchain.GetImageViews()[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = mPipelineBuilder.GetRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = mSwapchain.GetExtent().width;
        framebufferInfo.height = mSwapchain.GetExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(mDevice.GetLogical(), &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void App::InitCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = mDevice.GetQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(mDevice.GetLogical(), &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void App::InitCommandBuffers()
{
    mCommandBuffers.resize(M_MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) mCommandBuffers.size();

    if (vkAllocateCommandBuffers(mDevice.GetLogical(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void App::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffers!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mPipelineBuilder.GetRenderPass();
    renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = mSwapchain.GetExtent();
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.Get());
    
    VkBuffer vertexBuffers[] = {mVertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(mSwapchain.GetExtent().width);
    viewport.height = static_cast<float>(mSwapchain.GetExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = mSwapchain.GetExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, static_cast<uint32_t>(mVertices.size()), 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void App::CreateVertexBuffer()
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(mVertices[0]) * mVertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(mDevice.GetLogical(), &bufferInfo, nullptr, &mVertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mDevice.GetLogical(), mVertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(mDevice.GetLogical(), &allocInfo, nullptr, &mVertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(mDevice.GetLogical(), mVertexBuffer, mVertexBufferMemory, 0);

    void* data;
    vkMapMemory(mDevice.GetLogical(), mVertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, mVertices.data(), (size_t) bufferInfo.size);
    vkUnmapMemory(mDevice.GetLogical(), mVertexBufferMemory);
}

uint32_t App::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mDevice.GetPhysical(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

bool a = false;

void App::Update()
{
    Draw();
}

void App::Draw()
{
    vkWaitForFences(mDevice.GetLogical(), 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(mDevice.GetLogical(), 1, &mInFlightFences[mCurrentFrame]);
    
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mDevice.GetLogical(), mSwapchain.Get(), UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
    RecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphores[mCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphores[mCurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mDevice.GetGraphicsQueue(), 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!"); 
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {mSwapchain.Get()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional
    vkQueuePresentKHR(mDevice.GetGraphicsQueue(), &presentInfo);

    mCurrentFrame = (mCurrentFrame + 1) % M_MAX_FRAMES_IN_FLIGHT;
}

} // namespace tlr
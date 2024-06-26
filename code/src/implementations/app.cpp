#include "app.hpp"

namespace tlr
{

App::App()
{
    InitFramebuffers();
    InitCommandPool();
    InitCommandBuffers();
    InitSyncObjects();
}

App::~App()
{
    vkDeviceWaitIdle(StateBoard::device->GetLogical());

    for (auto framebuffer : mFramebuffers)
    {
        vkDestroyFramebuffer(StateBoard::device->GetLogical(), framebuffer, nullptr);
    }
    vkDestroyCommandPool(StateBoard::device->GetLogical(), mCommandPool, nullptr);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(StateBoard::device->GetLogical(), mSynchronizationPrimitites.imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(StateBoard::device->GetLogical(), mSynchronizationPrimitites.renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(StateBoard::device->GetLogical(), mSynchronizationPrimitites.inFlightFences[i], nullptr);
    }
}

void App::InitFramebuffers()
{
    mFramebuffers.resize(StateBoard::swapchain->GetImageViews().size());

    for (int i = 0; i < StateBoard::swapchain->GetImageViews().size(); i++)
    {
        
        VkImageView attachments[] = { StateBoard::swapchain->GetImageViews()[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = mPipeline.GetRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = StateBoard::swapchain->GetExtent().width;
        framebufferInfo.height = StateBoard::swapchain->GetExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(StateBoard::device->GetLogical(), &framebufferInfo, nullptr, &mFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void App::InitCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = StateBoard::device->GetQueueFamilyIndices();

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(StateBoard::device->GetLogical(), &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void App::InitCommandBuffers()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkAllocateCommandBuffers(StateBoard::device->GetLogical(), &allocInfo, &mCommandBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
}

void App::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mPipeline.GetRenderPass();
    renderPassInfo.framebuffer = mFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = StateBoard::swapchain->GetExtent();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline.Get());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(StateBoard::swapchain->GetExtent().width);
    viewport.height = static_cast<float>(StateBoard::swapchain->GetExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = StateBoard::swapchain->GetExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void App::InitSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if (vkCreateSemaphore(StateBoard::device->GetLogical(), &semaphoreInfo, nullptr, &mSynchronizationPrimitites.imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(StateBoard::device->GetLogical(), &semaphoreInfo, nullptr, &mSynchronizationPrimitites.renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(StateBoard::device->GetLogical(), &fenceInfo, nullptr, &mSynchronizationPrimitites.inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

void App::DrawFrame()
{
    vkWaitForFences(StateBoard::device->GetLogical(), 1, &mSynchronizationPrimitites.inFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(StateBoard::device->GetLogical(), 1, &mSynchronizationPrimitites.inFlightFences[mCurrentFrame]);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(StateBoard::device->GetLogical(), StateBoard::swapchain->Get(), UINT64_MAX, mSynchronizationPrimitites.imageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(mCommandBuffers[mCurrentFrame], 0);
    RecordCommandBuffer(mCommandBuffers[mCurrentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mSynchronizationPrimitites.imageAvailableSemaphores[mCurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];

    VkSemaphore signalSemaphores[] = {mSynchronizationPrimitites.renderFinishedSemaphores[mCurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    if (vkQueueSubmit(StateBoard::device->GetGraphicsQueue(), 1, &submitInfo, mSynchronizationPrimitites.inFlightFences[mCurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {StateBoard::swapchain->Get()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(StateBoard::device->GetPresentQueue(), &presentInfo);
    
    mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void App::CreateVertexBuffer()
{
    std::vector<Vertex> vertexBuffer
    {
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };
    uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size());

    std::vector<uint32_t> indexBuffer{ 0, 1, 2 };
    indices.count = static_cast<uint32_t>(indexBuffer.size());
    uint32_t indexBufferSize = indices.count * sizeof(uint32_t);

    VkMemoryAllocateInfo memAlloc{};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;
}

} // namespace tlr
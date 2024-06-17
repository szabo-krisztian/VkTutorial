#include "app.hpp"

namespace tlr
{

App::App()
{
    glfwInit();
    InitSyncObjects();
    InitFrameBuffers();
    InitCommandPool();
    InitCommandBuffer();
}

App::~App()
{
    vkDestroySemaphore(mDevice.GetLogical(), mImageAvailableSemaphore, nullptr);
    vkDestroySemaphore(mDevice.GetLogical(), mRenderFinishedSemaphore, nullptr);
    vkDestroyFence(mDevice.GetLogical(), mInFlightFence, nullptr);

    vkDestroyCommandPool(mDevice.GetLogical(), mCommandPool, nullptr);

    for (auto framebuffer : mSwapchainFramebuffers)
    {
        vkDestroyFramebuffer(mDevice.GetLogical(), framebuffer, nullptr);
    }
    
    glfwTerminate();
}

void App::Run()
{
    while (mWindow.IsWindowActive())
    {
        glfwPollEvents();
        Draw();
    }
    vkDeviceWaitIdle(mDevice.GetLogical());
}

void App::InitSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(mDevice.GetLogical(), &semaphoreInfo, nullptr, &mImageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(mDevice.GetLogical(), &semaphoreInfo, nullptr, &mRenderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(mDevice.GetLogical(), &fenceInfo, nullptr, &mInFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }
}

void App::InitFrameBuffers()
{
    mSwapchainFramebuffers.resize(mSwapchain.GetImageViews().size());

    for (size_t i = 0; i < mSwapchain.GetImageViews().size(); i++) {
        VkImageView attachments[] = {
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

void App::InitCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(mDevice.GetLogical(), &allocInfo, &mCommandBuffer) != VK_SUCCESS)
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
        throw std::runtime_error("failed to begin recording command buffer!");
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

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void App::Draw()
{
    vkWaitForFences(mDevice.GetLogical(), 1, &mInFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(mDevice.GetLogical(), 1, &mInFlightFence);
    
    uint32_t imageIndex;
    vkAcquireNextImageKHR(mDevice.GetLogical(), mSwapchain.Get(), UINT64_MAX, mImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(mCommandBuffer, 0);
    RecordCommandBuffer(mCommandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {mImageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffer;

    VkSemaphore signalSemaphores[] = {mRenderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mDevice.GetGraphicsQueue(), 1, &submitInfo, mInFlightFence) != VK_SUCCESS)
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
}

} // namespace tlr
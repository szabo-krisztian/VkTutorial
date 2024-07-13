#include "app.hpp"
#include "toolset.hpp"
#include "initializers.hpp"
#include "shader_module.hpp"

namespace tlr
{

App::App()
{
    InitQueues();
    
    InitCommands();
    InitSyncStructures();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
}

App::~App()
{
    vkDeviceWaitIdle(device);
    deleteQueue.flush();
}


App::FrameData& App::GetCurrentFrameData()
{
    return _frames[_frameNumber % FRAME_OVERLAP];
}

void App::InitQueues()
{
    _queues.graphicsQueueFamily = physicalDevice.familyIndices.graphicsFamily.value();
    vkGetDeviceQueue(device, _queues.graphicsQueueFamily, 0, &_queues.graphicsQueue);
    _queues.presentationQueueFamily = physicalDevice.familyIndices.presentFamily.value();
    vkGetDeviceQueue(device, _queues.presentationQueueFamily, 0, &_queues.presentationQueue);
}

void App::InitCommands()
{
    VkCommandPoolCreateInfo commandPoolCI = init::CommandPoolCreateInfo(_queues.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCI, nullptr, &_frames[i].commandPool));
        
        deleteQueue.push_function([this, i]() {
            vkDestroyCommandPool(device, _frames[i].commandPool, nullptr);
        });

        VkCommandBufferAllocateInfo commandBufferAI = init::CommandBufferAllocateInfo(_frames[i].commandPool, 1);
        VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAI, &_frames[i].mainCommandBuffer));
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

        deleteQueue.push_function([this, i]() {
            vkDestroySemaphore(device, _frames[i].renderSemaphore, nullptr);
            vkDestroySemaphore(device, _frames[i].swapchainSemaphore, nullptr);
            vkDestroyFence(device, _frames[i].renderFence, nullptr);
        });
    }
}

void App::CreateGraphicsPipeline()
{
    // Create shaderStages[]
    ShaderModule vertModule(device.device, "./spvs/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    ShaderModule fragModule(device.device, "./spvs/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertModule.GetCreateInfo(), fragModule.GetCreateInfo()};

    // Dynamic states
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicStateCI = init::PipelineDynamicStateCreateInfo(dynamicStates);

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputCI = init::PipelineVertexInputStateCreateInfo();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = init::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);    

    // Viewport state
    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);
    VkPipelineViewportStateCreateInfo viewportStateCI = init::PipelineViewportStateCreateInfo(1, 1, 0);
    viewportStateCI.pViewports = &viewport;
    viewportStateCI.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = init::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = init::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = init::PipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlending = init::PipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCI = init::PipelineLayoutCreateInfo((uint32_t)0);
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &_pipelineLayout));
    deleteQueue.push_function([=]() {
        vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
    });

    // Pipeline
    VkGraphicsPipelineCreateInfo pipelineCI = init::PipelineCreateInfo(_pipelineLayout, _renderPass);
    pipelineCI.stageCount = 2;
    pipelineCI.pStages = shaderStages;
    pipelineCI.pVertexInputState = &vertexInputCI;
    pipelineCI.pInputAssemblyState = &inputAssemblyCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pRasterizationState = &rasterizer;
    pipelineCI.pMultisampleState = &multisampling;
    pipelineCI.pDepthStencilState = nullptr;
    pipelineCI.pColorBlendState = &colorBlending;
    pipelineCI.pDynamicState = &dynamicStateCI;
    pipelineCI.subpass = 0;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &_graphicsPipeline));
    deleteQueue.push_function([this]() {
        vkDestroyPipeline(device, _graphicsPipeline, nullptr);
    });
}

void App::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchain.imageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCI{};
    renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCI.attachmentCount = 1;
    renderPassCI.pAttachments = &colorAttachment;
    renderPassCI.subpassCount = 1;
    renderPassCI.pSubpasses = &subpass;
    renderPassCI.dependencyCount = 1;
    renderPassCI.pDependencies = &dependency;
    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassCI, nullptr, &_renderPass));
    deleteQueue.push_function([this]() {
        vkDestroyRenderPass(device, _renderPass, nullptr);
    });
}

void App::CreateFramebuffers()
{
    _framebuffers.resize(swapchain.imageCount);
    for (int i = 0; i < _framebuffers.size(); ++i)
    {
        VkImageView attachments[] = {swapchain.imageViews[i]};
        VkFramebufferCreateInfo framebufferCI = init::FramebufferCreateInfo(_renderPass, 1, attachments, swapchain.extent.width, swapchain.extent.height, 1);
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferCI, nullptr, &_framebuffers[i]));
        deleteQueue.push_function([this, i](){
            vkDestroyFramebuffer(device, _framebuffers[i], nullptr);
        });
    }
}

void App::RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = init::CommandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &beginInfo));

    VkRenderPassBeginInfo renderPassInfo = init::RenderPassBeginInfo(_renderPass, _framebuffers[imageIndex]);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain.extent;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    // Drawing command
    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Drawing command
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);

    // Drawing command
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    // Drawing command
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // Drawing command
    vkCmdDraw(cmd, 3, 1, 0, 0);
    vkCmdEndRenderPass(cmd);
    VK_CHECK_RESULT(vkEndCommandBuffer(cmd));
}

void App::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        DrawFrame();
    }
}

void App::DrawFrame()
{
    auto frameData = GetCurrentFrameData();
    vkWaitForFences(device, 1, &frameData.renderFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &frameData.renderFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, frameData.swapchainSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    vkResetCommandBuffer(frameData.mainCommandBuffer, 0);
    RecordCommandBuffer(frameData.mainCommandBuffer, imageIndex);

    VkSemaphore waitSemaphores[] = {frameData.swapchainSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {frameData.renderSemaphore};
    VkSubmitInfo submitInfo = init::SubmitInfo(1, waitSemaphores, waitStages, 1, &frameData.mainCommandBuffer, 1, signalSemaphores);
    VK_CHECK_RESULT(vkQueueSubmit(_queues.graphicsQueue, 1, &submitInfo, frameData.renderFence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain.swapchain;
    presentInfo.pImageIndices = &imageIndex;
    vkQueuePresentKHR(_queues.presentationQueue, &presentInfo);

    ++_frameNumber;
}

} // namespace tlr
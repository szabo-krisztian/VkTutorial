#include "app.hpp"

#include <cstring>

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
    camera.SetMovementSpeed(5.0f);
    
    inputManager->AddKeyPressListener(GLFW_MOUSE_BUTTON_RIGHT, [&]() {
        _world.BuildBlock(camera.GetPosition(), camera.GetForwardVector());
    });
    inputManager->AddKeyPressListener(GLFW_MOUSE_BUTTON_LEFT, [&]() {
        _world.BreakBlock(camera.GetPosition(), camera.GetForwardVector());
    });

    InitCommands();
    InitSyncStructures();

    InitVertexBuffer();
    InitIndexBuffer();

    CreateDescriptorSetLayout();
    CreateDescriptorPool();
    CreateDescriptorSets();
    
    CreateGraphicsPipeline();
}

App::~App()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
}

void App::Update()
{
    UpdateDesciptorUbos();

    auto frameData = GetCurrentFrameData();
    vkWaitForFences(device, 1, &frameData.renderFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &frameData.renderFence);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, frameData.swapchainSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    vkResetCommandBuffer(frameData.commandBuffer, 0);    
    RecordCommandBuffer(frameData.commandBuffer, imageIndex);
    
    VkSemaphore waitSemaphores[] = {frameData.swapchainSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {frameData.renderSemaphore};
    VkSubmitInfo submitInfo = init::SubmitInfo(1, waitSemaphores, waitStages, 1, &frameData.commandBuffer, 1, signalSemaphores);
    VK_CHECK_RESULT(vkQueueSubmit(device.queues.graphics, 1, &submitInfo, frameData.renderFence));

    VkPresentInfoKHR presentInfo = init::PresentInfoKHR(1, signalSemaphores, &swapchain.swapchain, &imageIndex);
    vkQueuePresentKHR(device.queues.present, &presentInfo);
    _frameNumber = (_frameNumber + 1) % FRAME_OVERLAP;
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

void App::InitIndexBuffer()
{
    _cube.indices =
    {
        0, 1, 2, 2, 3, 0,

        7, 6, 5, 7, 5, 4,

        1, 6, 2, 1, 5, 6,

        4, 0, 3, 4, 3, 7,

        3, 2, 6, 3, 6, 7,

        4, 5, 1, 4, 1, 0
    };

    VkDeviceSize bufferSize = sizeof(_cube.indices[0]) * _cube.indices.size();
    Buffer stagingBuffer;
    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, bufferSize));
    VK_CHECK_RESULT(stagingBuffer.Map());
    stagingBuffer.CopyTo(_cube.indices.data(), bufferSize);
    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_cube.indexBuffer, bufferSize));
    ENQUEUE_OBJ_DEL(( [this]() { _cube.indexBuffer.Destroy(); } ));
    CopyBuffer(stagingBuffer.buffer, _cube.indexBuffer.buffer, bufferSize);
    stagingBuffer.Destroy();
}

void App::InitVertexBuffer()
{
    _cube.vertices =
    {
        {{0, 0, 0}},
        {{1, 0, 0}},
        {{1, 1, 0}},
        {{0, 1, 0}},
        {{0, 0, 1}},
        {{1, 0, 1}},
        {{1, 1, 1}},
        {{0, 1, 1}}
    };

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

void App::CreateDescriptorSets()
{
    _layout0.sets.resize(FRAME_OVERLAP);
    _layout0.ubos.resize(FRAME_OVERLAP);

    for (std::size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        auto& ubo = _layout0.ubos[i];
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ubo.cameraTransform, sizeof(CameraTransform)));
        ENQUEUE_OBJ_DEL(( [&] { ubo.cameraTransform.Destroy(); } ));
        VK_CHECK_RESULT(ubo.cameraTransform.Map());
    }

    uint32_t layout0DescriptorSetsCount = static_cast<uint32_t>(FRAME_OVERLAP);
    std::vector<VkDescriptorSetLayout> layouts0(layout0DescriptorSetsCount, _layout0);
    VkDescriptorSetAllocateInfo set0AllocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, layouts0.data(), layout0DescriptorSetsCount);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &set0AllocInfo, _layout0.sets.data()));

    for (std::size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        auto& set = _layout0.sets[i];
        auto& ubo = _layout0.ubos[i];

        VkWriteDescriptorSet descriptorWrites[] =
        {
            init::WriteDescriptorSet(set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &ubo.cameraTransform.descriptor),
        };
        vkUpdateDescriptorSets(device, 1, descriptorWrites, 0, nullptr);
    }
}

void App::UpdateDesciptorUbos()
{
    CameraTransform transform;
    transform.view = camera.GetViewMatrix();
    transform.proj = camera.GetProjectionMatrix();
    memcpy(_layout0.ubos[_frameNumber].cameraTransform.mapped, &transform, sizeof(CameraTransform));
}

void App::CreateGraphicsPipeline()
{
    // Create shaderStages[]
    std::string fragSpvPath = GetAbsolutePath("../build/spvs/frag.spv");
    std::string vertSpvPath = GetAbsolutePath("../build/spvs/vert.spv");

    ShaderModule vertModule(device.device, vertSpvPath, VK_SHADER_STAGE_VERTEX_BIT);
    ShaderModule fragModule(device.device, fragSpvPath, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertModule.GetCreateInfo(), fragModule.GetCreateInfo()};

    // Dynamic states
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicStateCI = init::PipelineDynamicStateCreateInfo(dynamicStates, 0);

    // Vertex input
    auto bindingDescription = VertexInfo::GetBindingDescription();
    auto attributeDescriptions = VertexInfo::GetAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputCI = init::PipelineVertexInputStateCreateInfo();
    vertexInputCI.vertexBindingDescriptionCount = 1;
    vertexInputCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputCI.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCI.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = init::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

    // Depth stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    // Viewport state
    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);
    VkPipelineViewportStateCreateInfo viewportStateCI = init::PipelineViewportStateCreateInfo(1, &viewport, 1, &scissor, 0);

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = init::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = init::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending = init::PipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    VkPushConstantRange pushConstant;
	pushConstant.offset = 0;
	pushConstant.size = sizeof(CubeInfo);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineLayoutCI.pPushConstantRanges = &pushConstant;
	pipelineLayoutCI.pushConstantRangeCount = 1;
    
    pipelineLayoutCI.setLayoutCount = 1;
    VkDescriptorSetLayout layout[] = {_layout0};
    pipelineLayoutCI.pSetLayouts = layout;
    
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &_pipelineLayout));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyPipelineLayout(device, _pipelineLayout, nullptr); } ));

    // Pipeline
    VkGraphicsPipelineCreateInfo pipelineCI = init::PipelineCreateInfo();
    pipelineCI.stageCount = 2;
    pipelineCI.pStages = shaderStages;
    pipelineCI.pVertexInputState = &vertexInputCI;
    pipelineCI.pInputAssemblyState = &inputAssemblyCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pRasterizationState = &rasterizer;
    pipelineCI.pMultisampleState = &multisampling;
    pipelineCI.pColorBlendState = &colorBlending;
    pipelineCI.pDynamicState = &dynamicStateCI;
    pipelineCI.pDepthStencilState = &depthStencil;
    pipelineCI.renderPass = renderPass;
    pipelineCI.layout = _pipelineLayout;
    pipelineCI.subpass = 0;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &_graphicsPipeline));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyPipeline(device, _graphicsPipeline, nullptr); } ));
}

void App::RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = init::CommandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &beginInfo));

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{1.0f, 1.0f, 1.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    VkRenderPassBeginInfo renderPassInfo = init::RenderPassBeginInfo(renderPass, framebuffers[imageIndex]);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain.extent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();    
    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_layout0.sets[_frameNumber], 0, nullptr);
    
    VkBuffer vertexBuffers[] = {_cube.vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, _cube.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

    std::vector<Block> blocks = _world.GetActiveBlocks();
    
    for (const auto& block : blocks)
    {
        CubeInfo pushConstant;
        pushConstant.color = block.GetColor();
        pushConstant.transform = block.GetModelMatrix();
        
        vkCmdPushConstants(cmd, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(CubeInfo), &pushConstant);
        vkCmdDrawIndexed(cmd, static_cast<uint32_t>(_cube.indices.size()), 1, 0, 0, 0);
    }
    
    vkCmdEndRenderPass(cmd);
    VK_CHECK_RESULT(vkEndCommandBuffer(cmd));
}

} // namespace tlr
#include "app.hpp"

#include <cstring>
#include <chrono>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "initializers.hpp"
#include "toolset.hpp"
#include "shader_module.hpp"


namespace tlr
{

App::App()
{
    InitQueues();
    InitCommands();
    InitSyncStructures();
    CreateVertexBuffer();
    CreateIndexBuffer();
    
    
    CreateUniformBuffers();
    CreateDescriptorSetLayout();

    CreateDescriptorPool();
    CreateDescriptorSets();
    
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
}

App::~App()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
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
    VkCommandPoolCreateInfo transferCommandPoolCI = init::CommandPoolCreateInfo(_queues.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    VK_CHECK_RESULT(vkCreateCommandPool(device, &transferCommandPoolCI, nullptr, &_transferPool));
    _deletionQueue.PushFunction([this] {
        vkDestroyCommandPool(device, _transferPool, nullptr);
    });


    VkCommandPoolCreateInfo commandPoolCI = init::CommandPoolCreateInfo(_queues.graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCI, nullptr, &_frames[i].commandPool));
        _deletionQueue.PushFunction([this, i] {
            vkDestroyCommandPool(device, _frames[i].commandPool, nullptr);
        });

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

        _deletionQueue.PushFunction([this, i]() {
            vkDestroySemaphore(device, _frames[i].renderSemaphore, nullptr);
            vkDestroySemaphore(device, _frames[i].swapchainSemaphore, nullptr);
            vkDestroyFence(device, _frames[i].renderFence, nullptr);
        });
    }
}

void App::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, _vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertexBuffer, _vertexBufferMemory);
    _deletionQueue.PushFunction([this]() {
        vkDestroyBuffer(device, _vertexBuffer, nullptr);
        vkFreeMemory(device, _vertexBufferMemory, nullptr);
    });
    CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void App::CreateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, _indices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indicesBuffer, _indicesBufferMemory);
    _deletionQueue.PushFunction([this]() {
        vkDestroyBuffer(device, _indicesBuffer, nullptr);
        vkFreeMemory(device, _indicesBufferMemory, nullptr);
    });
    CopyBuffer(stagingBuffer, _indicesBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void App::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    _uniformBuffers.resize(FRAME_OVERLAP);
    _uniformBuffersMemory.resize(FRAME_OVERLAP);
    _uniformBuffersMapped.resize(FRAME_OVERLAP);

    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffers[i], _uniformBuffersMemory[i]);
        vkMapMemory(device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);

        _deletionQueue.PushFunction([this, i]() {
            vkDestroyBuffer(device, _uniformBuffers[i], nullptr);
            vkFreeMemory(device, _uniformBuffersMemory[i], nullptr);
        });
    }
}

 void App::UpdateUniformBuffer(uint32_t currentImage)
 {
    static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::translate(glm::mat4(1.0), glm::vec3(std::cos(time * glm::radians(180.0f)), std::sin(time * glm::radians(180.0f)), 0.0f));

        ubo.model *= glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapchain.extent.width / (float) swapchain.extent.height, 0.1f, 10.0f);
        

        memcpy(_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
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
    vkQueueSubmit(_queues.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_queues.graphicsQueue);

    vkFreeCommandBuffers(device, _transferPool, 1, &commandBuffer);
}

void App::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = init::BufferCreateInfo(usage, size);

    VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
    VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory));
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

uint32_t App::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);   
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void App::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(FRAME_OVERLAP);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool));
    _deletionQueue.PushFunction([this]() {
        vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
    });
}

void App::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(FRAME_OVERLAP, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(FRAME_OVERLAP);
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(FRAME_OVERLAP);
    if (vkAllocateDescriptorSets(device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void App::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;  // Add this line

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_descriptorSetLayout));
    _deletionQueue.PushFunction([this]() {
        vkDestroyDescriptorSetLayout(device, _descriptorSetLayout, nullptr);
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
    _deletionQueue.PushFunction([this]() {
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
        _deletionQueue.PushFunction([this, i](){
            vkDestroyFramebuffer(device, _framebuffers[i], nullptr);
        });
    }
}

std::string GetAbsolutePath(const std::string& relativePath)
{
    std::string fullPath(__FILE__);
    std::string directory = fullPath.substr(0, fullPath.find_last_of("\\/") + 1);
    std::string absolutePath = directory + relativePath;
    return absolutePath;
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
    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputCI = init::PipelineVertexInputStateCreateInfo();
    vertexInputCI.vertexBindingDescriptionCount = 1;
    vertexInputCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputCI.pVertexBindingDescriptions = &bindingDescription;
    vertexInputCI.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = init::PipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

    // Viewport state
    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);
    VkPipelineViewportStateCreateInfo viewportStateCI = init::PipelineViewportStateCreateInfo(1, &viewport, 1, &scissor, 0);

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer = init::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling = init::PipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment = init::PipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlending = init::PipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts = &_descriptorSetLayout;
    

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &_pipelineLayout));
    _deletionQueue.PushFunction([&]() {
        vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
    });

    // Pipeline
    VkGraphicsPipelineCreateInfo pipelineCI = init::PipelineCreateInfo();
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
    pipelineCI.renderPass = _renderPass;
    pipelineCI.layout = _pipelineLayout;
    pipelineCI.subpass = 0;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &_graphicsPipeline));
    _deletionQueue.PushFunction([&]() {
        vkDestroyPipeline(device, _graphicsPipeline, nullptr);
    });
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

    // Drawing command
    VkBuffer vertexBuffers[] = {_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    // Drawing command
    vkCmdBindIndexBuffer(cmd, _indicesBuffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);

    // Drawing command
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    // Drawing command
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[_frameNumber % FRAME_OVERLAP], 0, nullptr);
    vkCmdDrawIndexed(cmd, static_cast<uint32_t>(_indices.size()), 1, 0, 0, 0);

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
    UpdateUniformBuffer(_frameNumber % FRAME_OVERLAP);
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
    VK_CHECK_RESULT(vkQueueSubmit(_queues.graphicsQueue, 1, &submitInfo, frameData.renderFence));

    VkPresentInfoKHR presentInfo = init::PresentInfoKHR(1, signalSemaphores, &swapchain.swapchain, &imageIndex);
    vkQueuePresentKHR(_queues.presentationQueue, &presentInfo);
    ++_frameNumber;
}

} // namespace tlr
#include "app.hpp"

#include <cstring>
#include <chrono>
#include <cmath>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "initializers.hpp"
#include "toolset.hpp"
#include "shader_module.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (_deletionQueue).PushFunction(lambda)

namespace tlr
{

App::App()
{
    inputManager->AddKeyPressListener(GLFW_KEY_E, [&]() { ShootBullet(); });

    InitCommands();
    InitSyncStructures();

    CreateCubeVertices();
    CreateCubeVertexBuffer();
    CreateBulletVertices();
    CreateBulletVertexBuffer();
    

    CreateDescriptorPool();
    CreateCameraTransformDescriptorSetLayout();
    CreateCameraTransformUniformBuffers();
    CreateCameraTransformDescriptorSets();
    CreateModelTransformDescriptorSetLayout();
    CreateCubeTransformUniformBuffers();
    CreateCubeTransformDescriptorSets();
    CreateBulletTransformsUniformBuffers();
    CreateBulletTransformsDescriptorSets();


    CreateDepthResources();
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
    return _frames[_frameNumber];
}

void App::InitCommands()
{
    VkCommandPoolCreateInfo transferCommandPoolCI = init::CommandPoolCreateInfo(device.queues.graphicsFamily, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    VK_CHECK_RESULT(vkCreateCommandPool(device, &transferCommandPoolCI, nullptr, &_transferPool));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyCommandPool(device, _transferPool, nullptr); } ));

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

        ENQUEUE_OBJ_DEL(( [this, i]() {
            vkDestroySemaphore(device, _frames[i].renderSemaphore, nullptr);
            vkDestroySemaphore(device, _frames[i].swapchainSemaphore, nullptr);
            vkDestroyFence(device, _frames[i].renderFence, nullptr);
        } ));
    }
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

void App::CreateCubeVertices()
{
    physx::PxVec3 cubeDimensions(15.0f, 15.0f, 15.0f);
    physx::PxVec3 cubePosition(0.0f, 100.0f, 0.0f);
    physx::PxU32  numberOfVertices = 20;

    _simulator.CreateMainMesh(cubePosition, cubeDimensions, numberOfVertices);
    for (const auto& pos : _simulator.GetMainMeshTriangles())
    {
        glm::vec3 randomColor = {util::RandomFloat(0.0f, 1.0f), 0, util::RandomFloat(0.0f, 1.0f)};
        _boxVertices.push_back(Vertex{pos, randomColor});
    }
}

void App::CreateCubeVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(_boxVertices[0]) * _boxVertices.size();
    
    Buffer stagingBuffer;    
    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, bufferSize));
    VK_CHECK_RESULT(stagingBuffer.Map());
    stagingBuffer.CopyTo(_boxVertices.data(), bufferSize);

    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_boxVertexBuffer, bufferSize));
    ENQUEUE_OBJ_DEL(( [this]() { _boxVertexBuffer.Destroy(); } ));

    CopyBuffer(stagingBuffer.buffer, _boxVertexBuffer.buffer, bufferSize);

    stagingBuffer.Destroy();
}

void App::CreateBulletVertices()
{
    PxVec3 bulletDimensions(2.0f, 2.0f, 2.0f);
    PxU32  numberOfVertices = 7;

    _simulator.CreateBulletMesh(bulletDimensions, numberOfVertices);
    for (const auto& pos : _simulator.GetBulletMeshTriangles())
    {
        glm::vec3 randomColor = {util::RandomFloat(0.0f, .5f), util::RandomFloat(0.0f, .5f), util::RandomFloat(0.0f, .5f)};
        _bulletVertices.push_back(Vertex{pos, randomColor});
    }
}

void App::CreateBulletVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(_bulletVertices[0]) * _bulletVertices.size();

    Buffer stagingBuffer;
    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, bufferSize));
    VK_CHECK_RESULT(stagingBuffer.Map());
    stagingBuffer.CopyTo(_bulletVertices.data(), bufferSize);

    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_bulletVertexBuffer, bufferSize));
    ENQUEUE_OBJ_DEL(( [this]() { _bulletVertexBuffer.Destroy(); } ));

    CopyBuffer(stagingBuffer.buffer, _bulletVertexBuffer.buffer, bufferSize);

    stagingBuffer.Destroy();
}



void App::CreateDescriptorPool()
{
    uint32_t cameraTransformDescriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize cameraTransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cameraTransformDescriptorCount);

    uint32_t cubeTransformDescriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize cubeTransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cubeTransformDescriptorCount);

    uint32_t bulletTransformsDescriptorCount = static_cast<uint32_t>(FRAME_OVERLAP) * static_cast<uint32_t>(BULLET_COUNT);
    VkDescriptorPoolSize bulletTransformsSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bulletTransformsDescriptorCount);

    uint32_t maxDescriptorCount = cameraTransformDescriptorCount + cubeTransformDescriptorCount + bulletTransformsDescriptorCount;
    VkDescriptorPoolSize poolsizes[] = {cameraTransformSize, cubeTransformSize, bulletTransformsSize};
    VkDescriptorPoolCreateInfo poolInfo = init::DescriptorPoolCreateInfo(3, poolsizes, maxDescriptorCount);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorPool(device, _descriptorPool, nullptr); } ));
}

void App::CreateCameraTransformDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
    VkDescriptorSetLayoutCreateInfo layoutInfo = init::DescriptorSetLayoutCreateInfo(1, &uboLayoutBinding);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_cameraTransform.layout));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorSetLayout(device, _cameraTransform.layout, nullptr); } ));
}

void App::CreateCameraTransformUniformBuffers()
{
    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_cameraTransform.ubos[i], sizeof(UniformBufferObject)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _cameraTransform.ubos[i].Destroy(); } ));
        VK_CHECK_RESULT(_cameraTransform.ubos[i].Map());   
    }
}

void App::CreateCameraTransformDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(FRAME_OVERLAP, _cameraTransform.layout);
    VkDescriptorSetAllocateInfo allocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, layouts.data(), static_cast<uint32_t>(FRAME_OVERLAP));
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, _cameraTransform.sets));

    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VkWriteDescriptorSet descriptorWrite = init::WriteDescriptorSet(_cameraTransform.sets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &_cameraTransform.ubos[i].descriptor);
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void App::UpdateCameraTransform(uint32_t currentImage)
{
    UniformBufferObject ubo{};
    ubo.view = camera.GetViewMatrix();
    ubo.proj = camera.GetProjectionMatrix();
    memcpy(_cameraTransform.ubos[currentImage].mapped, &ubo, sizeof(ubo));
}

void App::CreateModelTransformDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
    VkDescriptorSetLayoutCreateInfo layoutInfo = init::DescriptorSetLayoutCreateInfo(1, &uboLayoutBinding);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_modelTransformLayout));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorSetLayout(device, _modelTransformLayout, nullptr); } ));
}

void App::CreateCubeTransformUniformBuffers()
{
    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_cubeTransform.ubos[i], sizeof(glm::mat4)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _cubeTransform.ubos[i].Destroy(); } ));
        VK_CHECK_RESULT(_cubeTransform.ubos[i].Map());   
    }
}

void App::CreateCubeTransformDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(FRAME_OVERLAP, _modelTransformLayout);
    VkDescriptorSetAllocateInfo allocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, layouts.data(), static_cast<uint32_t>(FRAME_OVERLAP));

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, _cubeTransform.sets));

    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VkWriteDescriptorSet descriptorWrite = init::WriteDescriptorSet(_cubeTransform.sets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &_cubeTransform.ubos[i].descriptor);
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void App::UpdateCubeTransform(uint32_t currentImage)
{
    glm::mat4 ubo = _simulator.GetMainMeshTransform();
    memcpy(_cubeTransform.ubos[currentImage].mapped, &ubo, sizeof(ubo));
}

void App::CreateBulletTransformsUniformBuffers()
{
    for (size_t i = 0; i < FRAME_OVERLAP * BULLET_COUNT; i++) 
    {
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_bulletTransforms  .ubos[i], sizeof(glm::mat4)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _bulletTransforms.ubos[i].Destroy(); } ));
        VK_CHECK_RESULT(_bulletTransforms.ubos[i].Map());   
    }
}

void App::CreateBulletTransformsDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(FRAME_OVERLAP * BULLET_COUNT, _modelTransformLayout);
    VkDescriptorSetAllocateInfo allocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, layouts.data(), static_cast<uint32_t>(FRAME_OVERLAP * BULLET_COUNT));
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, _bulletTransforms.sets));

    for (size_t i = 0; i < FRAME_OVERLAP * BULLET_COUNT; i++)
    {
        VkWriteDescriptorSet descriptorWrite = init::WriteDescriptorSet(_bulletTransforms.sets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &_bulletTransforms.ubos[i].descriptor);
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void App::UpdateBulletTransforms(uint32_t currentImage)
{
    _bulletTransforms.count = _simulator.GetBulletCount();
    std::vector<glm::mat4> bulletTransforms = _simulator.GetBulletTransforms();
    for (int i = 0; i < _bulletTransforms.count; ++i)
    {
        memcpy(_bulletTransforms.ubos[currentImage * BULLET_COUNT + i].mapped, &bulletTransforms[i], sizeof(glm::mat4));
    }
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

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassInfo, nullptr, &_renderPass));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyRenderPass(device, _renderPass, nullptr); } ));
}

void App::CreateFramebuffers()
{
    _framebuffers.resize(swapchain.imageCount);
    for (int i = 0; i < _framebuffers.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = {
            swapchain.imageViews[i],
            _depthBuffer.depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo = init::FramebufferCreateInfo();
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchain.extent.width;
        framebufferInfo.height = swapchain.extent.height;
        framebufferInfo.height = swapchain.extent.height;
        framebufferInfo.layers = 1;
        
        VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &_framebuffers[i]));
        ENQUEUE_OBJ_DEL(( [this, i]() { vkDestroyFramebuffer(device, _framebuffers[i], nullptr); } ));
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
    VkPipelineColorBlendAttachmentState colorBlendAttachment = init::PipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlending = init::PipelineColorBlendStateCreateInfo(1, &colorBlendAttachment);

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    pipelineLayoutCI.setLayoutCount = 2;
    VkDescriptorSetLayout layout[] = {_cameraTransform.layout, _modelTransformLayout};
    pipelineLayoutCI.pSetLayouts = layout;
    

    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &_pipelineLayout));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyPipelineLayout(device, _pipelineLayout, nullptr); } ));

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
    pipelineCI.pDepthStencilState = &depthStencil;
    pipelineCI.renderPass = _renderPass;
    pipelineCI.layout = _pipelineLayout;
    pipelineCI.subpass = 0;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &_graphicsPipeline));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyPipeline(device, _graphicsPipeline, nullptr); } ));
}

void App::RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = init::CommandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &beginInfo));

    VkRenderPassBeginInfo renderPassInfo = init::RenderPassBeginInfo(_renderPass, _framebuffers[imageIndex]);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain.extent;
    
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{1.0f, 1.0f, 1.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    
    
    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    VkBuffer vertexBuffers[] = {_boxVertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_cameraTransform.sets[_frameNumber], 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_cubeTransform.sets[_frameNumber], 0, nullptr);
    vkCmdDraw(cmd, static_cast<uint32_t>(_boxVertices.size()), 1, 0, 0);

    if (_bulletTransforms.count > 0)
    {
        VkBuffer vertexBuffers2[] = {_bulletVertexBuffer.buffer};
        VkDeviceSize offsets2[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers2, offsets2);
    }
    for (int i = 0; i < _simulator.GetBulletCount(); ++i)
    {
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_bulletTransforms.sets[_frameNumber * BULLET_COUNT + i], 0, nullptr);
        vkCmdDraw(cmd, static_cast<uint32_t>(_bulletVertices.size()), 1, 0, 0);
    }

    vkCmdEndRenderPass(cmd);
    VK_CHECK_RESULT(vkEndCommandBuffer(cmd));
}

void App::ShootBullet()
{    
    auto velocity = util::GlmVec3ToPxVec3(camera.GetForwardVector());
    auto position = util::GlmVec3ToPxVec3(camera.GetPosition());
    float bulletSpeed = 100.0f;
    _simulator.ShootBullet(velocity * bulletSpeed, position + velocity);
}

void App::Update()
{
    _simulator.Update(timer.GetDeltaTime());

    UpdateCameraTransform(_frameNumber);
    UpdateCubeTransform(_frameNumber);
    UpdateBulletTransforms(_frameNumber);

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



void App::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat();
    CreateImage(swapchain.extent.width, swapchain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthBuffer.depthImage, _depthBuffer.depthImageMemory);
    _depthBuffer.depthImageView = CreateImageView(_depthBuffer.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    ENQUEUE_OBJ_DEL(( [this]() {
        vkDestroyImage(device, _depthBuffer.depthImage, nullptr);
        vkFreeMemory(device, _depthBuffer.depthImageMemory, nullptr);
        vkDestroyImageView(device, _depthBuffer.depthImageView, nullptr);
    } ));
}

VkFormat App::FindDepthFormat()
{
    return FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

bool HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat App::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

void App::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo = init::ImageCreateInfo();
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = init::MemoryAllocateInfo();
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device.GetMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory));

    vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView App::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo = init::ImageViewCreateInfo();
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VK_CHECK_RESULT(vkCreateImageView(device, &viewInfo, nullptr, &imageView));

    return imageView;
}

} // namespace tlr
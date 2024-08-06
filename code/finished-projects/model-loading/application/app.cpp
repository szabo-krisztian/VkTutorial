#include "app.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "initializers.hpp"
#include "toolset.hpp"
#include "shader_module.hpp"

#define ENQUEUE_OBJ_DEL(lambda) (_deletionQueue).PushFunction(lambda)

namespace tlr
{

App::App()
{
    InitCommands();
    InitSyncStructures();

    ReadMeshInfo();
    InitMeshVertexBuffer();


    CreateDescriptorLayouts();
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
    auto pos = camera.GetPosition();
    std::cout << pos.x << ", " << pos.y << ", " << pos.z << std::endl;

    UpdateDesciptorUbos(_frameNumber);

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

        ENQUEUE_OBJ_DEL(( [this, i]() {
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


void App::ReadMeshInfo()
{
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = MTL_PATH;

    if (!reader.ParseFromFile(MODEL_PATH, reader_config))
    {
        if (!reader.Error().empty())
        {
            std::cerr << "TinyObjReader: " << reader.Error() << std::endl;
        }
        exit(1);
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader: " << reader.Warning() << std::endl;
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    for (const auto& m : materials)
    {
        Material material;
        material.specularExponent = m.shininess;
        material.ambient = glm::vec3(m.ambient[0], m.ambient[1], m.ambient[2]);
        material.diffuse = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
        material.specular = glm::vec3(m.specular[0], m.specular[1], m.specular[2]);
        material.emissive = glm::vec3(m.emission[0], m.emission[1], m.emission[2]);
        material.alpha = m.dissolve;
        _mesh.materials.push_back(material);
    }

    for (const auto& shape : shapes)
    {
        const auto& mesh = shape.mesh;

        size_t index_offset = 0;
        for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f)
        {
            size_t fv = mesh.num_face_vertices[f];
            int materialId = mesh.material_ids[f];

            for (size_t v = 0; v < fv; ++v)
            {
                tinyobj::index_t idx = mesh.indices[index_offset + v];
                
                glm::vec3 position =
                {
                    attrib.vertices[3 * size_t(idx.vertex_index) + 0],
                    attrib.vertices[3 * size_t(idx.vertex_index) + 1],
                    attrib.vertices[3 * size_t(idx.vertex_index) + 2]
                };

                glm::vec3 normal =
                {
                    attrib.normals[3 * size_t(idx.normal_index) + 0],
                    attrib.normals[3 * size_t(idx.normal_index) + 1],
                    attrib.normals[3 * size_t(idx.normal_index) + 2]
                };

                Vertex vertex = {position, normal};
                
                if (_mesh.vertices.find(materialId) == _mesh.vertices.end())
                {
                    _mesh.vertices[materialId] = std::vector<Vertex>();
                }

                _mesh.vertices[materialId].push_back(vertex);
            }
            index_offset += fv;
        }
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

void App::InitMeshVertexBuffer()
{
    _mesh.buffers.resize(_mesh.materials.size());

    for (int i = 0; i < _mesh.materials.size(); ++i)
    {
        auto& vertices = _mesh.vertices[i];
        auto& buffer = _mesh.buffers[i];

        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        Buffer stagingBuffer;
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, bufferSize));
        VK_CHECK_RESULT(stagingBuffer.Map());
        stagingBuffer.CopyTo(vertices.data(), bufferSize);

        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buffer, bufferSize));
        ENQUEUE_OBJ_DEL(([this, i] { _mesh.buffers[i].Destroy(); }));

        CopyBuffer(stagingBuffer.buffer, buffer.buffer, bufferSize);

        stagingBuffer.Destroy();
    }
}


void App::CreateDescriptorLayouts()
{
    VkDescriptorSetLayoutBinding modelBindingSet0 = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
    VkDescriptorSetLayoutBinding lightBindingSet0 = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
    VkDescriptorSetLayoutBinding cameraPositionBindingSet0 = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);
    VkDescriptorSetLayoutBinding bindingsSet0[] = { modelBindingSet0, lightBindingSet0, cameraPositionBindingSet0 };
    VkDescriptorSetLayoutCreateInfo layoutInfoSet0 = init::DescriptorSetLayoutCreateInfo(3, bindingsSet0);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfoSet0, nullptr, &_set0Layout));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorSetLayout(device, _set0Layout, nullptr); } ));

    VkDescriptorSetLayoutBinding materialBindingSet1 = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1);
    VkDescriptorSetLayoutBinding bindingsSet1[] = { materialBindingSet1 };
    VkDescriptorSetLayoutCreateInfo layoutInfoSet1 = init::DescriptorSetLayoutCreateInfo(1, bindingsSet1);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfoSet1, nullptr, &_set1Layout));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorSetLayout(device, _set1Layout, nullptr); } ));
}

void App::CreateDescriptorPool()
{
    uint32_t modelTransformCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize modeltransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, modelTransformCount);

    uint32_t lightCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize lightSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lightCount);
    
    uint32_t cameraPositionCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize cameraPositionSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cameraPositionCount);
    
    uint32_t materialsCount = static_cast<uint32_t>(FRAME_OVERLAP * _mesh.materials.size());
    VkDescriptorPoolSize materialsSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, materialsCount);
    
    uint32_t maxDescriptorCount = modelTransformCount + lightCount + cameraPositionCount + materialsCount;
    VkDescriptorPoolSize poolsizes[] = {modeltransformSize, lightSize, cameraPositionSize, materialsSize};
    VkDescriptorPoolCreateInfo poolInfo = init::DescriptorPoolCreateInfo(4, poolsizes, maxDescriptorCount);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorPool(device, _descriptorPool, nullptr); } ));
}

void App::CreateDescriptorSets()
{
    _sets0.resize(FRAME_OVERLAP);
    _sets1.resize(FRAME_OVERLAP * _mesh.materials.size());

    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        auto& set = _sets0[i];

        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &set.modelUbo, sizeof(ModelTransform)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _sets0[i].modelUbo.Destroy(); } ));
        VK_CHECK_RESULT(set.modelUbo.Map());

        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &set.lightUbo, sizeof(Light)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _sets0[i].lightUbo.Destroy(); } ));
        VK_CHECK_RESULT(set.lightUbo.Map());
        
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &set.cameraPositionUbo, sizeof(glm::vec3)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _sets0[i].cameraPositionUbo.Destroy(); } ));
        VK_CHECK_RESULT(set.cameraPositionUbo.Map())
    }

    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        for (size_t j = 0; j < _mesh.materials.size(); ++j)
        {
            auto& set = _sets1[i * _mesh.materials.size() + j];

            VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &set.materialUbo, sizeof(Material)));
            ENQUEUE_OBJ_DEL(( [this, i, j]() { _sets1[i * _mesh.materials.size() + j].materialUbo.Destroy(); } ));
            VK_CHECK_RESULT(set.materialUbo.Map());
        }
    }
    
    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        VkDescriptorSetAllocateInfo set0AllocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, &_set0Layout, 1);
        VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &set0AllocInfo, &_sets0[i].set));


        VkWriteDescriptorSet descriptorWrites[] =
        {
            init::WriteDescriptorSet(_sets0[i].set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &_sets0[i].modelUbo.descriptor),
            init::WriteDescriptorSet(_sets0[i].set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &_sets0[i].lightUbo.descriptor),
            init::WriteDescriptorSet(_sets0[i].set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &_sets0[i].cameraPositionUbo.descriptor)
        };
        vkUpdateDescriptorSets(device, 3, descriptorWrites, 0, nullptr);

        assert(_sets0[i].set != VK_NULL_HANDLE);
    }
    
    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        for (size_t j = 0; j < _mesh.materials.size(); ++j)
        {
            VkDescriptorSetAllocateInfo set1AllocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, &_set1Layout, 1);
            VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &set1AllocInfo, &_sets1[i * _mesh.materials.size() + j].set));

            
            VkWriteDescriptorSet descriptorWrites[] =
            {
                init::WriteDescriptorSet(_sets1[i * _mesh.materials.size() + j].set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &_sets1[i * _mesh.materials.size() + j].materialUbo.descriptor)
            };
            vkUpdateDescriptorSets(device, 1, descriptorWrites, 0, nullptr);
        }
    }
}

void App::UpdateDesciptorUbos(uint32_t frameNumber)
{
    ModelTransform modelUbo {};
    modelUbo.vertexTransform = glm::mat4(1.0f);
    modelUbo.normalTransform = glm::mat4(1.0f);
    modelUbo.view = camera.GetViewMatrix();
    modelUbo.proj = camera.GetProjectionMatrix();
    memcpy(_sets0[frameNumber].modelUbo.mapped, &modelUbo, sizeof(ModelTransform));

    Light lightUbo {};
    lightUbo.position = {-0.0999182, 22.4754, -1.63288};
    lightUbo.lightColor = {1,1,1};
    lightUbo.lightPower = 70.0f;
    memcpy(_sets0[frameNumber].lightUbo.mapped, &lightUbo, sizeof(Light));

    glm::vec3 cameraUbo = camera.GetPosition();
    memcpy(_sets0[frameNumber].cameraPositionUbo.mapped, &cameraUbo, sizeof(glm::vec3));

    for (size_t j = 0; j < _mesh.materials.size(); ++j)
    {
        Material materialUbo = _mesh.materials[j];
        memcpy(_sets1[frameNumber * _mesh.materials.size() + j].materialUbo.mapped, &materialUbo, sizeof(Material));
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
    pipelineLayoutCI.setLayoutCount = 2;
    VkDescriptorSetLayout layout[] = {_set0Layout, _set1Layout};
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
    pipelineCI.pColorBlendState = &colorBlending;
    pipelineCI.pDynamicState = &dynamicStateCI;
    pipelineCI.pDepthStencilState = &depthStencil;
    pipelineCI.renderPass = renderPass;
    pipelineCI.layout = _pipelineLayout;
    pipelineCI.subpass = 0;
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &_graphicsPipeline));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyPipeline(device, _graphicsPipeline, nullptr); } ));
}


void App::RecordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = init::CommandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &beginInfo));

    VkRenderPassBeginInfo renderPassInfo = init::RenderPassBeginInfo(renderPass, framebuffers[imageIndex]);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain.extent;
    
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    
    
    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_sets0[_frameNumber].set, 0, nullptr);
    for (size_t i = 0; i < _mesh.materials.size(); ++i)
    {
        size_t index = _frameNumber * _mesh.materials.size() + i;

        VkBuffer vertexBuffers[] = {_mesh.buffers[i].buffer};
        VkDeviceSize offsets[] = {0};
        
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_sets1[index].set, 0, nullptr);
        vkCmdDraw(cmd, static_cast<uint32_t>(_mesh.vertices[i].size()), 1, 0, 0);
    }

    vkCmdEndRenderPass(cmd);
    VK_CHECK_RESULT(vkEndCommandBuffer(cmd));
}

} // namespace tlr
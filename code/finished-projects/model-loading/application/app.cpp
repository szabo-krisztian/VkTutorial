#include "app.hpp"

#include <cstring>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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

App::App() :
    MODEL_PATH{GetAbsolutePath("bugatti/bugatti.obj")},
    MTL_PATH{GetAbsolutePath("bugatti")}
{
    camera.SetPosition({3.82992f, 7.52581f, 23.5453f});
    camera.SetLookAtPoint({0.458236f, 4.42813f, 1.57407f});

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
    _mesh.materialsCount = materials.size();
    
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
    _mesh.buffers.resize(_mesh.materialsCount);

    for (int i = 0; i < _mesh.materialsCount; ++i)
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
    
    VkDescriptorSetLayoutBinding bindingsSet0[] =
    {
        modelBindingSet0, lightBindingSet0, cameraPositionBindingSet0
    };
    VkDescriptorSetLayoutCreateInfo layoutInfoSet0 = init::DescriptorSetLayoutCreateInfo(3, bindingsSet0);

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfoSet0, nullptr, _layout0));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyDescriptorSetLayout(device, _layout0, nullptr); } ));

    VkDescriptorSetLayoutBinding materialBindingSet1 = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1);
    VkDescriptorSetLayoutBinding bindingsSet1[] =
    {
        materialBindingSet1
    };
    VkDescriptorSetLayoutCreateInfo layoutInfoSet1 = init::DescriptorSetLayoutCreateInfo(1, bindingsSet1);

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfoSet1, nullptr, _layout1));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyDescriptorSetLayout(device, _layout1, nullptr); } ));
}

void App::CreateDescriptorPool()
{
    uint32_t modelTransformCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize modeltransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, modelTransformCount);

    uint32_t lightCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize lightSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lightCount);
    
    uint32_t cameraPositionCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize cameraPositionSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cameraPositionCount);
    
    uint32_t materialsCount = static_cast<uint32_t>(FRAME_OVERLAP * _mesh.materialsCount);
    VkDescriptorPoolSize materialsSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, materialsCount);
    
    uint32_t maxDescriptorCount = modelTransformCount + lightCount + cameraPositionCount + materialsCount;
    VkDescriptorPoolSize poolsizes[] = {modeltransformSize, lightSize, cameraPositionSize, materialsSize};
    VkDescriptorPoolCreateInfo poolInfo = init::DescriptorPoolCreateInfo(4, poolsizes, maxDescriptorCount);

    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool));
    ENQUEUE_OBJ_DEL(( [this] { vkDestroyDescriptorPool(device, _descriptorPool, nullptr); } ));
}

void App::CreateDescriptorSets()
{
    _layout0.sets.resize(FRAME_OVERLAP);
    _layout1.sets.resize(FRAME_OVERLAP * _mesh.materialsCount);
    _layout0.ubos.resize(FRAME_OVERLAP);
    _layout1.ubos.resize(FRAME_OVERLAP * _mesh.materialsCount);

    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        auto& ubo = _layout0.ubos[i];

        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ubo.model, sizeof(ModelTransform)));
        ENQUEUE_OBJ_DEL(( [&] { ubo.model.Destroy(); } ));
        VK_CHECK_RESULT(ubo.model.Map());

        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ubo.light, sizeof(Light)));
        ENQUEUE_OBJ_DEL(( [&] { ubo.light.Destroy(); } ));
        VK_CHECK_RESULT(ubo.light.Map());
        
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ubo.cameraPosition, sizeof(glm::vec3)));
        ENQUEUE_OBJ_DEL(( [&] { ubo.cameraPosition.Destroy(); } ));
        VK_CHECK_RESULT(ubo.cameraPosition.Map())
    }

    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        for (size_t j = 0; j < _mesh.materialsCount; ++j)
        {
            auto& ubo = _layout1.ubos[i * _mesh.materialsCount + j];

            VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ubo.material, sizeof(Material)));
            ENQUEUE_OBJ_DEL(( [&] { ubo.material.Destroy(); } ));
            VK_CHECK_RESULT(ubo.material.Map());
        }
    }
    
    uint32_t layout0DescriptorSetsCount = static_cast<uint32_t>(FRAME_OVERLAP);
    std::vector<VkDescriptorSetLayout> layouts0(layout0DescriptorSetsCount, _layout0);
    VkDescriptorSetAllocateInfo set0AllocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, layouts0.data(), layout0DescriptorSetsCount);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &set0AllocInfo, _layout0.sets.data()));

    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        auto& set = _layout0.sets[i];
        auto& ubo = _layout0.ubos[i];

        VkWriteDescriptorSet descriptorWrites[] =
        {
            init::WriteDescriptorSet(set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &ubo.model.descriptor),
            init::WriteDescriptorSet(set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &ubo.light.descriptor),
            init::WriteDescriptorSet(set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &ubo.cameraPosition.descriptor)
        };
        vkUpdateDescriptorSets(device, 3, descriptorWrites, 0, nullptr);
    }

    uint32_t layout1DescriptorSetsCount = FRAME_OVERLAP * static_cast<uint32_t>(_mesh.materialsCount);
    std::vector<VkDescriptorSetLayout> layouts1(layout1DescriptorSetsCount, _layout1);
    VkDescriptorSetAllocateInfo set1AllocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, layouts1.data(), layout1DescriptorSetsCount);
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &set1AllocInfo, _layout1.sets.data()));

    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        for (size_t j = 0; j < _mesh.materialsCount; ++j)
        {    
            auto& set = _layout1.sets[i * _mesh.materialsCount + j];
            auto& ubo = _layout1.ubos[i * _mesh.materialsCount + j];

            VkWriteDescriptorSet descriptorWrites[] =
            {
                init::WriteDescriptorSet(set, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &ubo.material.descriptor)
            };
            vkUpdateDescriptorSets(device, 1, descriptorWrites, 0, nullptr);
        }
    }
}

void App::UpdateDesciptorUbos()
{
    ModelTransform modelUbo {};
    modelUbo.vertexTransform = glm::mat4(1.0f);
    modelUbo.normalTransform = glm::mat4(1.0f);
    modelUbo.view = camera.GetViewMatrix();
    modelUbo.proj = camera.GetProjectionMatrix();
    memcpy(_layout0.ubos[_frameNumber].model.mapped, &modelUbo, sizeof(ModelTransform));

    Light lightUbo {};
    glm::vec3 pos{-0.753088, 9.57204,-1.55403};
    glm::vec3 circle{std::cos(timer.GetElapsedTime()) * 10.0f, 0.0, std::sin(timer.GetElapsedTime()) * 10.0f};
    lightUbo.position = pos + circle;
    lightUbo.lightColor = {1,1,1};
    lightUbo.lightPower = 20.0f;
    memcpy(_layout0.ubos[_frameNumber].light.mapped, &lightUbo, sizeof(Light));

    glm::vec3 cameraUbo = camera.GetPosition();
    memcpy(_layout0.ubos[_frameNumber].cameraPosition.mapped, &cameraUbo, sizeof(glm::vec3));

    for (size_t j = 0; j < _mesh.materialsCount; ++j)
    {
        Material materialUbo = _mesh.materials[j];
        memcpy(_layout1.ubos[_frameNumber * _mesh.materialsCount + j].material.mapped, &materialUbo, sizeof(Material));
    }
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
    VkDescriptorSetLayout layout[] = {_layout0, _layout1};
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
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
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
    for (int i = 0; i < _mesh.materialsCount; ++i)
    {
        VkBuffer vertexBuffers[] = {_mesh.buffers[i].buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        size_t index = _frameNumber * _mesh.materialsCount + i;
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_layout1.sets[index], 0, nullptr);

        vkCmdDraw(cmd, static_cast<uint32_t>(_mesh.vertices[i].size()), 1, 0, 0);
    }

    vkCmdEndRenderPass(cmd);
    VK_CHECK_RESULT(vkEndCommandBuffer(cmd));
}

} // namespace tlr

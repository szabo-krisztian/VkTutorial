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
    CreateMainMeshVertexBuffer();

    CreateDescriptorPool();
    CreateCameraTransformDescriptorSetLayout();
    CreateCameraTransformUniformBuffers();
    CreateCameraTransformDescriptorSets();

    CreateModelTransformDescriptorSetLayout();
    CreateMainMeshTransformUniformBuffers();
    CreateMainMeshTransformDescriptorSets();

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
    auto forw = camera.GetForwardVector();

    std::cout << "pos:" << pos.x << ", "<< pos.y << ", " << pos.z << std::endl;
    std::cout << "forw:" << forw.x << ", "<< forw.y << ", " << forw.z << std::endl;

    UpdateCameraTransform(_frameNumber);
    UpdateOtherSets(_frameNumber);
    UpdateMainMeshTransform(_frameNumber);
    

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

App::FrameData& App::GetCurrentFrameData()
{
    return _frames[_frameNumber];
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

    // Initialize materials
    for (const auto& mat : materials)
    {
        Material material;
        
        // Specular Exponent (Ns)
        material.specularExponent = mat.shininess;

        // Ambient (Ka)
        material.ambient = glm::vec3(mat.ambient[0], mat.ambient[1], mat.ambient[2]);

        // Diffuse (Kd)
        material.diffuse = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

        // Specular (Ks)
        material.specular = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);

        // Emissive (Ke)
        material.emissive = glm::vec3(mat.emission[0], mat.emission[1], mat.emission[2]);

        // Alpha (d)
        material.alpha = mat.dissolve;

        _materials.push_back(material);
    }

    // Loop over shapes
    for (const auto& shape : shapes)
    {
        const auto& mesh = shape.mesh;

        // Loop over faces (polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f)
        {
            size_t fv = mesh.num_face_vertices[f];
            int materialId = mesh.material_ids[f];

            // Ensure the material ID is valid
            if (materialId < 0 || materialId >= _materials.size())
            {
                std::cerr << "Warning: Invalid material ID " << materialId << std::endl;
                continue;
            }

            // Initialize the vector if it does not exist
            if (_shapeVertices.find(materialId) == _shapeVertices.end())
            {
                _shapeVertices[materialId] = std::vector<Vertex>();
            }

            // Loop over vertices in the face
            for (size_t v = 0; v < fv; ++v)
            {
                // Access to vertex
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
                
                _shapeVertices[materialId].push_back(vertex);
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

void App::CreateMainMeshVertexBuffer()
{
    for (const auto& pair : _shapeVertices)
    {
        int materialID = pair.first;
        auto vertices = pair.second;

        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        Buffer stagingBuffer;    
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, bufferSize));
        VK_CHECK_RESULT(stagingBuffer.Map());
        stagingBuffer.CopyTo(vertices.data(), bufferSize);

        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_shapeBuffers[materialID], bufferSize));
        
        ENQUEUE_OBJ_DEL(([this, materialID] { _shapeBuffers[materialID].Destroy(); }));

        CopyBuffer(stagingBuffer.buffer, _shapeBuffers[materialID].buffer, bufferSize);

        stagingBuffer.Destroy();
    }
}

void App::CreateDescriptorPool()
{
    uint32_t cameraTransformDescriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize cameraTransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cameraTransformDescriptorCount);

    uint32_t mainMeshTransformDescriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize mainMeshTransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mainMeshTransformDescriptorCount);

    uint32_t matDescriptorCount = static_cast<uint32_t>(FRAME_OVERLAP * _materials.size());
    VkDescriptorPoolSize matSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, matDescriptorCount);

    uint32_t lightDescCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize lightSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, lightDescCount);

    uint32_t camData = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize camSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, camData);

    uint32_t maxDescriptorCount = cameraTransformDescriptorCount + mainMeshTransformDescriptorCount + matDescriptorCount + lightDescCount + camData;
    VkDescriptorPoolSize poolsizes[] = {cameraTransformSize, mainMeshTransformSize, matSize, lightSize, camSize};
    VkDescriptorPoolCreateInfo poolInfo = init::DescriptorPoolCreateInfo(5, poolsizes, maxDescriptorCount);

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
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_cameraTransform.ubos[i], sizeof(CameraTransform)));
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
    CameraTransform ubo{};
    ubo.view = camera.GetViewMatrix();
    ubo.proj = camera.GetProjectionMatrix();
    memcpy(_cameraTransform.ubos[currentImage].mapped, &ubo, sizeof(ubo));
}

void App::CreateModelTransformDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding modelTransformBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
    VkDescriptorSetLayoutBinding lightBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);
    VkDescriptorSetLayoutBinding cameraBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);
    
    VkDescriptorSetLayoutBinding bindings[] = {modelTransformBinding, lightBinding, cameraBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = init::DescriptorSetLayoutCreateInfo(3, bindings);
    
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layoutSet1));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorSetLayout(device, layoutSet1, nullptr); } ));

    VkDescriptorSetLayoutBinding materialBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1);
    VkDescriptorSetLayoutBinding bindingsMat[] = {materialBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfoMat = init::DescriptorSetLayoutCreateInfo(1, bindingsMat);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfoMat, nullptr, &layoutSet2));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorSetLayout(device, layoutSet2, nullptr); } ));
}

void App::CreateMainMeshTransformUniformBuffers()
{
    _mats.buffers.resize(FRAME_OVERLAP * _materials.size());

    for (size_t i = 0; i < FRAME_OVERLAP; ++i)
    {
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_model.buffers[i], sizeof(ModelTransform)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _model.buffers[i].Destroy(); } ));
        VK_CHECK_RESULT(_model.buffers[i].Map());

        for (size_t j = 0; j < _materials.size(); ++j)
        {
            VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_mats.buffers[i * _materials.size() + j], sizeof(Material)));
            
            ENQUEUE_OBJ_DEL(( [this, i, j]() { _mats.buffers[i * _materials.size() + j].Destroy(); } ));
            VK_CHECK_RESULT(_mats.buffers[i * _materials.size() + j].Map());   
            memcpy(_mats.buffers[i * _materials.size() + j].mapped, &_materials[j], sizeof(Material));
            _mats.buffers[i * _materials.size() + j].Unmap();   
        }
        

        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_light.buffers[i], sizeof(Light)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _light.buffers[i].Destroy(); } ));
        VK_CHECK_RESULT(_light.buffers[i].Map());   
        Light light;
        light.position = {0,0,0};
        light.lightColor = {1,1,1};
        light.lightPower = 60.0f;
        memcpy(_light.buffers[i].mapped, &light, sizeof(Light));
        _light.buffers[i].Unmap();


        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_cam.buffers[i], sizeof(glm::vec3)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _cam.buffers[i].Destroy(); } ));
        VK_CHECK_RESULT(_cam.buffers[i].Map());   
    }
}

void App::CreateMainMeshTransformDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts1(FRAME_OVERLAP, layoutSet1);
    VkDescriptorSetAllocateInfo allocInfoModel = init::DescriptorSetAllocateInfo(_descriptorPool, layouts1.data(), static_cast<uint32_t>(FRAME_OVERLAP));
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfoModel, sets1));
    
    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VkWriteDescriptorSet descriptorWrites[] = {
            init::WriteDescriptorSet(sets1[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &_model.buffers[i].descriptor),
            init::WriteDescriptorSet(sets1[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &_light.buffers[i].descriptor),
            init::WriteDescriptorSet(sets1[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &_cam.buffers[i].descriptor)
        };
        vkUpdateDescriptorSets(device, 3, descriptorWrites, 0, nullptr);
    }

    sets2.resize(FRAME_OVERLAP * _materials.size());
    std::vector<VkDescriptorSetLayout> layouts2(FRAME_OVERLAP * _materials.size(), layoutSet2);
    VkDescriptorSetAllocateInfo allocInfoMat = init::DescriptorSetAllocateInfo(_descriptorPool, layouts2.data(), static_cast<uint32_t>(FRAME_OVERLAP) * static_cast<uint32_t>(_materials.size()));
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfoMat, sets2.data()));
    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        for (size_t j = 0; j < _materials.size(); ++j)
        {
            VkWriteDescriptorSet descriptorWrite = init::WriteDescriptorSet(sets2[i * _materials.size() + j], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &_mats.buffers[i * _materials.size() + j].descriptor);
            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }
    }
    
}

void App::UpdateMainMeshTransform(uint32_t currentImage)
{
    ModelTransform modelTransform {};
    modelTransform.vertex = glm::mat4(1.0f);
    modelTransform.normal = glm::transpose(glm::inverse(modelTransform.vertex));
    //model = glm::rotate(model, 3.14f / 2.0f * timer.GetElapsedTime(), glm::vec3(0.0f, 1.0f, 0.0f));
    memcpy(_model.buffers[currentImage].mapped, &modelTransform, sizeof(ModelTransform));
}

std::string GetAbsolutePath(const std::string& relativePath)
{
    std::string fullPath(__FILE__);
    std::string directory = fullPath.substr(0, fullPath.find_last_of("\\/") + 1);
    std::string absolutePath = directory + relativePath;
    return absolutePath;
}

void App::UpdateOtherSets(uint32_t currentImage)
{   
    glm::vec3 cameraPosition = camera.GetPosition();
    memcpy(_cam.buffers[currentImage].mapped, &cameraPosition, sizeof(glm::vec3));
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
    
    pipelineLayoutCI.setLayoutCount = 3;
    VkDescriptorSetLayout layout[] = {_cameraTransform.layout, layoutSet1, layoutSet2};
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

    for (const auto& pair : _shapeBuffers)
    {
        VkBuffer vertexBuffers[] = {pair.second.buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

        VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
        VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        
        VkDescriptorSet descriptorSets[] = {
            _cameraTransform.sets[_frameNumber],
            sets1[_frameNumber],
            sets2[_frameNumber * _materials.size() + pair.first],
        };

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 3, descriptorSets, 0, nullptr);


        vkCmdDraw(cmd, static_cast<uint32_t>(_shapeVertices[pair.first].size()), 1, 0, 0);
    }

    vkCmdEndRenderPass(cmd);
    VK_CHECK_RESULT(vkEndCommandBuffer(cmd));
}

} // namespace tlr
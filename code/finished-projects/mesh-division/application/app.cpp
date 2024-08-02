#include "app.hpp"

#include <random>
#include <utility>

#include <glm/gtc/matrix_transform.hpp>

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
    CreateDepthResources();

    CreateMainMeshVertices();
    CreateMainMeshVertexBuffer();

    CreateDescriptorPool();
    CreateCameraTransformDescriptorSetLayout();
    CreateCameraTransformUniformBuffers();
    CreateCameraTransformDescriptorSets();
    CreateModelTransformDescriptorSetLayout();
    CreateMainMeshTransformUniformBuffers();
    CreateMainMeshTransformDescriptorSets();

    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
}

App::~App()
{
    vkDeviceWaitIdle(device);
    _deletionQueue.Flush();
}

void App::Update()
{
    UpdateCameraTransform(_frameNumber);
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



namespace MeshDivision
{

struct Triangle
{
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 v3;

    glm::vec3 Center() const
    {
        return (v1+v2+v3) / 3.0f;
    }
};

struct Tetra
{
    glm::vec3 points[4];
};

bool IsClockwise(const Triangle& triangle, const glm::vec3& polycenter)
{
    auto trcenter = triangle.Center();
    auto outvector = trcenter - polycenter;
    
    auto v1_v2 = triangle.v2 - triangle.v1;
    auto v1_v3 = triangle.v3 - triangle.v1;
    auto normal = glm::cross(v1_v2, v1_v3);
    
    return glm::dot(normal, outvector) < 0;
}

glm::vec3 GetMidpoint(const glm::vec3& v1, const glm::vec3& v2)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    double random_number = dis(gen);
    glm::vec3 res_rand = v1 + static_cast<float>(random_number)*(v2 - v1);
    return res_rand;
}

std::vector<Tetra> DivideTetra(const Tetra& tetra)
{
    std::vector<Tetra> res;
    auto& A = tetra.points[0];
    auto& B = tetra.points[1];
    auto& C = tetra.points[2];
    auto& D = tetra.points[3];

    glm::vec3 A_B = GetMidpoint(A, B); // out    
    glm::vec3 A_C = GetMidpoint(A, C);
    glm::vec3 A_D = GetMidpoint(A, D);
    glm::vec3 B_C = GetMidpoint(B, C);
    glm::vec3 B_D = GetMidpoint(B, D);
    glm::vec3 C_D = GetMidpoint(C, D);  // in

    Tetra ACorner{A, A_B, A_C, A_D};
    Tetra BCorner{B, A_B, B_D, B_C};
    Tetra CCorner{C, B_C, A_C, C_D};
    Tetra DCorner{D, A_D, B_D, C_D};
    
    auto& near = A_B;
    auto& far = C_D;

    Tetra in1{near, far, A_C, A_D};
    Tetra in2{near, far, A_C, B_C};
    Tetra in3{near, far, B_C, B_D};
    Tetra in4{near, far, A_D, B_D};

    res.push_back(ACorner);
    res.push_back(BCorner);
    res.push_back(CCorner);
    res.push_back(DCorner);

    res.push_back(in1);
    res.push_back(in2);
    res.push_back(in3);
    res.push_back(in4);

    return res;
}

void DivideTetras(std::vector<Tetra>& tetras, int depth)
{
    for (int i = 0; i < depth; ++i)
    {
        size_t size = tetras.size();
        for (int j = 0; j < size; ++j)
        {
            auto divided = DivideTetra(tetras[j]);
            tetras[j] = divided[0];
            for (const auto& t : divided)
            {
                tetras.push_back(t);
            }
        }
    }
}

std::vector<glm::vec3> TrianglesFromTetra(const Tetra& tetra)
{
    auto& points = tetra.points;

    std::vector<Triangle> triangles(4);

    triangles[0] = {points[0], points[1], points[2]};
    triangles[1] = {points[0], points[1], points[3]};
    triangles[2] = {points[1], points[2], points[3]};
    triangles[3] = {points[0], points[2], points[3]};

    glm::vec3 polycenter = (points[0] + points[1] + points[2] + points[3]);
    glm::vec3 avg = polycenter / 4.0f;
    for (auto& triangle : triangles)
    {
        if (IsClockwise(triangle, avg))
        {
            auto temp = triangle.v1;
            triangle.v1 = triangle.v2;
            triangle.v2 = temp;
        }
    }

    std::vector<glm::vec3> res;
    for (const auto& triangle : triangles)
    {
        res.push_back(triangle.v1);
        res.push_back(triangle.v2);
        res.push_back(triangle.v3);
    }
    
    return res;
}

} // namespace MeshDivision

void App::CreateMainMeshVertices()
{
    float t = (1 + glm::sqrt(5.0f)) / 2.0f;
    std::vector<glm::vec3> vertices =
    {
        { t,    1.0f, 0.0f},
        {-t,    1.0f, 0.0f},
        { t,   -1.0f, 0.0f},
        {-t,   -1.0f, 0.0f},
        { 1.0f, 0.0f, t   },
        { 1.0f, 0.0f, -t   },
        {-1.0f, 0.0f, t   },
        {-1.0f, 0.0f,-t   },
        { 0.0f, t,    1.0f},
        { 0.0f,-t,    1.0f},
        { 0.0f, t,   -1.0f},
        { 0.0f,-t,   -1   }
    };
    std::vector<glm::vec3> triangles = 
    {
        {0, 8, 4},
        {0, 5, 10},
        {2, 4, 9},
        {2, 11, 5},
        {1, 6, 8},
        {1, 10, 7},
        {3, 9, 6},
        {2, 9, 11},
        {3, 9, 11},
        {4, 2, 0},
        {5, 0, 2},
        {6, 1, 3},
        {7, 3, 1},
        {8, 6, 4},
        {3, 7, 11},
        {0, 10, 8},
        {1, 8, 10},
        {9, 4, 6},
        {10, 5, 7},
        {11, 7, 5}
    };

    glm::vec3 center{0.0f, 0.0f, 0.0f};
    for (const auto& vertex : vertices)
    {
        center += vertex;
    }
    center /= vertices.size();

    std::vector<MeshDivision::Tetra> tetras;
    for (const auto& triangle : triangles)
    {
        auto v1 = vertices[static_cast<unsigned int>(triangle.x)];
        auto v2 = vertices[static_cast<unsigned int>(triangle.y)];
        auto v3 = vertices[static_cast<unsigned int>(triangle.z)];

        MeshDivision::Tetra tetra{v1, v2, v3, center};
        tetras.push_back(tetra);
    }

    
    MeshDivision::DivideTetras(tetras, 2);
    
    std::vector<glm::vec3> triangleVertices;
    for (const auto& tetra : tetras)
    {
        std::vector<glm::vec3> currentTriangleVertices = MeshDivision::TrianglesFromTetra(tetra);
        for (const auto& vertex : currentTriangleVertices)
        {
            triangleVertices.push_back(vertex);
        }
    }

    for (const auto& vertex : triangleVertices)
    {
        _mainMesh.vertices.push_back(Vertex{vertex, {1.0f, 0.0f, 0.0f}});
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
    VkDeviceSize bufferSize = sizeof(_mainMesh.vertices[0]) * _mainMesh.vertices.size();
    
    Buffer stagingBuffer;    
    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, bufferSize));
    VK_CHECK_RESULT(stagingBuffer.Map());
    stagingBuffer.CopyTo(_mainMesh.vertices.data(), bufferSize);

    VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &_mainMesh.vertexBuffer, bufferSize));
    ENQUEUE_OBJ_DEL(( [this]() { _mainMesh.vertexBuffer.Destroy(); } ));

    CopyBuffer(stagingBuffer.buffer, _mainMesh.vertexBuffer.buffer, bufferSize);

    stagingBuffer.Destroy();
}



void App::CreateDescriptorPool()
{
    uint32_t cameraTransformDescriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize cameraTransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, cameraTransformDescriptorCount);

    uint32_t mainMeshTransformDescriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);
    VkDescriptorPoolSize mainMeshTransformSize = init::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mainMeshTransformDescriptorCount);

    uint32_t maxDescriptorCount = cameraTransformDescriptorCount + mainMeshTransformDescriptorCount;
    VkDescriptorPoolSize poolsizes[] = {cameraTransformSize, mainMeshTransformSize};
    VkDescriptorPoolCreateInfo poolInfo = init::DescriptorPoolCreateInfo(2, poolsizes, maxDescriptorCount);

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
    VkDescriptorSetLayoutBinding uboLayoutBinding = init::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
    VkDescriptorSetLayoutCreateInfo layoutInfo = init::DescriptorSetLayoutCreateInfo(1, &uboLayoutBinding);
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &_modelTransformLayout));
    ENQUEUE_OBJ_DEL(( [this]() { vkDestroyDescriptorSetLayout(device, _modelTransformLayout, nullptr); } ));
}

void App::CreateMainMeshTransformUniformBuffers()
{
    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VK_CHECK_RESULT(device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_mainMesh.transformBuffers[i], sizeof(glm::mat4)));
        ENQUEUE_OBJ_DEL(( [this, i]() { _mainMesh.transformBuffers[i].Destroy(); } ));
        VK_CHECK_RESULT(_mainMesh.transformBuffers[i].Map());   
    }
}

void App::CreateMainMeshTransformDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(FRAME_OVERLAP, _modelTransformLayout);
    VkDescriptorSetAllocateInfo allocInfo = init::DescriptorSetAllocateInfo(_descriptorPool, layouts.data(), static_cast<uint32_t>(FRAME_OVERLAP));

    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocInfo, _mainMesh.transformSets));

    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VkWriteDescriptorSet descriptorWrite = init::WriteDescriptorSet(_mainMesh.transformSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &_mainMesh.transformBuffers[i].descriptor);
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

void App::UpdateMainMeshTransform(uint32_t currentImage)
{
    glm::mat4 model{1.0f};
    model = glm::scale(model, glm::vec3(10,10,10));
    model = glm::rotate(model, 3.14f / 8.0f * timer.GetElapsedTime(), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, 3.14f / 3, glm::vec3(0.0f, 0.0f, 1.0f));
    memcpy(_mainMesh.transformBuffers[currentImage].mapped, &model, sizeof(model));
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
    VkPipelineRasterizationStateCreateInfo rasterizer = init::PipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_LINE, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);

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

    VkBuffer vertexBuffers[] = {_mainMesh.vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);

    VkViewport viewport = init::Viewport(static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.0f, 1.0f);
    VkRect2D scissor = init::Rect2D({0, 0}, swapchain.extent);
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_cameraTransform.sets[_frameNumber], 0, nullptr);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 1, 1, &_mainMesh.transformSets[_frameNumber], 0, nullptr);
    vkCmdDraw(cmd, static_cast<uint32_t>(_mainMesh.vertices.size()), 1, 0, 0);

    vkCmdEndRenderPass(cmd);
    VK_CHECK_RESULT(vkEndCommandBuffer(cmd));
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
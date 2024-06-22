#include "pipeline_blueprint.hpp"

namespace tlr
{

PipelineBlueprint::PipelineBlueprint()
{
    InitDynamicStateInfo();
    InitInputAssemblyStateInfo();
    InitViewportStateInfo();
    InitRasterizerInfo();
    InitMultisamplingInfo();
    InitColorBlendingInfo();
    InitRenderPassInfo();
}

PipelineBlueprint::~PipelineBlueprint()
{
    vkDestroyPipeline(StateBoard::device->GetLogical(), mPipeline, nullptr);
    vkDestroyRenderPass(StateBoard::device->GetLogical(), mRenderPass, nullptr);
    vkDestroyPipelineLayout(StateBoard::device->GetLogical(), mPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(StateBoard::device->GetLogical(), mDescriptorSetLayout, nullptr);
    vkDestroyShaderModule(StateBoard::device->GetLogical(), mVertexModule, nullptr);
    vkDestroyShaderModule(StateBoard::device->GetLogical(), mFragmentModule, nullptr);
}

const VkPipeline& PipelineBlueprint::Get() const
{
    return mPipeline;
}

const VkPipelineLayout& PipelineBlueprint::GetLayout() const
{
    return mPipelineLayout;
}

const VkRenderPass& PipelineBlueprint::GetRenderPass() const
{
    return mRenderPass;
}

const VkDescriptorSetLayout& PipelineBlueprint::GetDescriptorLayout() const
{
    return mDescriptorSetLayout;
}

void PipelineBlueprint::SetVertexShaderSpvPath(const std::string& vertexSpvPath)
{
    std::vector<char> code = GetSpvBinary(vertexSpvPath);
    VkShaderModuleCreateInfo vertexModuleCreateInfo = {};
    vertexModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexModuleCreateInfo.pNext = nullptr;
    vertexModuleCreateInfo.flags = 0;
    vertexModuleCreateInfo.codeSize = code.size();
    vertexModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(StateBoard::device->GetLogical(), &vertexModuleCreateInfo, nullptr, &mVertexModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex shader module!");
    }

    mVertexShaderStageInfo = {};
    mVertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mVertexShaderStageInfo.pNext = nullptr;
    mVertexShaderStageInfo.flags = 0;
    mVertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    mVertexShaderStageInfo.module = mVertexModule;
    mVertexShaderStageInfo.pName = "main";
    mVertexShaderStageInfo.pSpecializationInfo = nullptr;
}

void PipelineBlueprint::SetFragmentShaderSpvPath(const std::string& fragmentSpvPath)
{
    std::vector<char> code = GetSpvBinary(fragmentSpvPath);
    VkShaderModuleCreateInfo fragmentModuleCreateInfo = {};
    fragmentModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentModuleCreateInfo.pNext = nullptr;
    fragmentModuleCreateInfo.flags = 0;
    fragmentModuleCreateInfo.codeSize = code.size();
    fragmentModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(StateBoard::device->GetLogical(), &fragmentModuleCreateInfo, nullptr, &mFragmentModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex shader module!");
    }

    mFragmentShaderStageInfo = {};
    mFragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mFragmentShaderStageInfo.pNext = nullptr;
    mFragmentShaderStageInfo.flags = 0;
    mFragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    mFragmentShaderStageInfo.module = mFragmentModule;
    mFragmentShaderStageInfo.pName = "main";
    mFragmentShaderStageInfo.pSpecializationInfo = nullptr;
}

void PipelineBlueprint::SetVertexInputState(uint32_t bindingCount, const VkVertexInputBindingDescription* bindingDesciptions, uint32_t attributeCount, const VkVertexInputAttributeDescription* attributeDescriptions)
{
    mVertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    mVertexInputStateInfo.pNext = nullptr;
    mVertexInputStateInfo.vertexBindingDescriptionCount = bindingCount;
    mVertexInputStateInfo.pVertexBindingDescriptions = bindingDesciptions;
    mVertexInputStateInfo.vertexAttributeDescriptionCount = attributeCount;
    mVertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions;
}

void PipelineBlueprint::SetLayout(const std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = descriptorBindings.size();
    layoutInfo.pBindings = descriptorBindings.data();

    if (vkCreateDescriptorSetLayout(StateBoard::device->GetLogical(), &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    mPipelineLayoutInfo = {};
    mPipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    mPipelineLayoutInfo.setLayoutCount = 1;
    mPipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
    mPipelineLayoutInfo.pushConstantRangeCount = 0;
    mPipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(StateBoard::device->GetLogical(), &mPipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void PipelineBlueprint::BuildGraphicsPipeline()
{
    if (vkCreateGraphicsPipelines(StateBoard::device->GetLogical(), VK_NULL_HANDLE, 1, &mGraphicsPipelineInfo, nullptr, &mPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void PipelineBlueprint::InitDynamicStateInfo()
{
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    mDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    mDynamicStateInfo.pNext = nullptr;
    mDynamicStateInfo.flags = 0;
    mDynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    mDynamicStateInfo.pDynamicStates = dynamicStates.data();
}

void PipelineBlueprint::InitInputAssemblyStateInfo()
{
    mInputAssemblyInfo = {};
    mInputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    mInputAssemblyInfo.pNext = nullptr;
    mInputAssemblyInfo.flags = 0;
    mInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    mInputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
}

void PipelineBlueprint::InitViewportStateInfo()
{
    mScissor = {};
    mScissor.offset = { 0, 0 };
    mScissor.extent = StateBoard::swapchain->GetExtent();

    mViewportStateInfo = {};
    mViewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    mViewportStateInfo.pNext = nullptr;
    mViewportStateInfo.flags = 0;
    mViewportStateInfo.viewportCount = 1;
    mViewportStateInfo.scissorCount = 1;
    mViewportStateInfo.pScissors = &mScissor;
}

void PipelineBlueprint::InitRasterizerInfo()
{
    mRasterizerInfo = {};
    mRasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    mRasterizerInfo.depthClampEnable = VK_FALSE;
    mRasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
    mRasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
    mRasterizerInfo.lineWidth = 1.0f;
    mRasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    mRasterizerInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    mRasterizerInfo.depthBiasEnable = VK_FALSE;
    mRasterizerInfo.depthBiasConstantFactor = 0.0f;
    mRasterizerInfo.depthBiasClamp = 0.0f;
    mRasterizerInfo.depthBiasSlopeFactor = 0.0f;
}

void PipelineBlueprint::InitMultisamplingInfo()
{
    mMultisamplingInfo = {};
    mMultisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    mMultisamplingInfo.sampleShadingEnable = VK_FALSE;
    mMultisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void PipelineBlueprint::InitColorBlendingInfo()
{
    mColorBlendAttachment = {};
    mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    mColorBlendAttachment.blendEnable = VK_FALSE;

    mColorBlendingInfo = {};
    mColorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    mColorBlendingInfo.logicOpEnable = VK_FALSE;
    mColorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
    mColorBlendingInfo.attachmentCount = 1;
    mColorBlendingInfo.pAttachments = &mColorBlendAttachment;
    mColorBlendingInfo.blendConstants[0] = 0.0f;
    mColorBlendingInfo.blendConstants[1] = 0.0f;
    mColorBlendingInfo.blendConstants[2] = 0.0f;
    mColorBlendingInfo.blendConstants[3] = 0.0f;
}

void PipelineBlueprint::InitRenderPassInfo()
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = StateBoard::swapchain->GetFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    mRenderPassInfo = {};
    mRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    mRenderPassInfo.attachmentCount = 1;
    mRenderPassInfo.pAttachments = &colorAttachment;
    mRenderPassInfo.subpassCount = 1;
    mRenderPassInfo.pSubpasses = &subpass;
    mRenderPassInfo.dependencyCount = 1;
    mRenderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(StateBoard::device->GetLogical(), &mRenderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void PipelineBlueprint::BuildPipelineCreateInfo()
{
    mGraphicsPipelineInfo = {};
    mGraphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    mGraphicsPipelineInfo.stageCount = 2;
    mGraphicsPipelineInfo.pStages = &mVertexShaderStageInfo;
    mGraphicsPipelineInfo.pVertexInputState = &mVertexInputStateInfo;
    mGraphicsPipelineInfo.pInputAssemblyState = &mInputAssemblyInfo;
    mGraphicsPipelineInfo.pViewportState = &mViewportStateInfo;
    mGraphicsPipelineInfo.pRasterizationState = &mRasterizerInfo;
    mGraphicsPipelineInfo.pMultisampleState = &mMultisamplingInfo;
    mGraphicsPipelineInfo.pDepthStencilState = nullptr;
    mGraphicsPipelineInfo.pColorBlendState = &mColorBlendingInfo;
    mGraphicsPipelineInfo.pDynamicState = &mDynamicStateInfo;
    mGraphicsPipelineInfo.layout = mPipelineLayout;
    mGraphicsPipelineInfo.renderPass = mRenderPass;
    mGraphicsPipelineInfo.subpass = 0;
    mGraphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    mGraphicsPipelineInfo.basePipelineIndex = -1;
}

std::vector<char> PipelineBlueprint::GetSpvBinary(const std::string& fileName)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

} // namespace tlr
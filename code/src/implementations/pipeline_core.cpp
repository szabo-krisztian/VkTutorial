#include "pipeline_core.hpp"

namespace tlr
{

PipelineCore::PipelineCore
(
    const std::string&                                    vertexSpvPath,
    const std::string&                                    fragmentSpvPath,
    const std::vector<VkVertexInputBindingDescription>&   bindingDesciptions,
    const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
    const std::vector<VkDescriptorSetLayoutBinding>&      descriptorBindings
)
{
    InitShaderInfos
    (
        vertexSpvPath,
        fragmentSpvPath
    );
    InitDynamicStateInfo();
    InitVertexInputStateInfo
    (
        bindingDesciptions,
        attributeDescriptions
    );
    InitInputAssemblyStateInfo();
    InitViewportStateInfo();
    InitRasterizerInfo();
    InitMultisamplingInfo();
    InitColorBlendingInfo();
    InitDescriptorSetLayoutInfo
    (
        descriptorBindings
    );
    InitRenderPassInfo();
    InitPipelineInfo();
}

PipelineCore::~PipelineCore()
{
    vkDestroyRenderPass(StateBoard::device->GetLogical(), mRenderPass, nullptr);
    vkDestroyPipelineLayout(StateBoard::device->GetLogical(), mPipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(StateBoard::device->GetLogical(), mDescriptorSetLayout, nullptr);
    vkDestroyShaderModule(StateBoard::device->GetLogical(), mVertexModule, nullptr);
    vkDestroyShaderModule(StateBoard::device->GetLogical(), mFragmentModule, nullptr);
}

std::vector<char> PipelineCore::GetSpvBinary(const std::string& fileName)
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

void PipelineCore::InitShaderInfos(const std::string& vertexSpvPath, const std::string& fragmentSpvPath)
{
    mVertexCode = GetSpvBinary(vertexSpvPath);
    VkShaderModuleCreateInfo vertexModuleCreateInfo = {};
    vertexModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexModuleCreateInfo.pNext = nullptr;
    vertexModuleCreateInfo.flags = 0;
    vertexModuleCreateInfo.codeSize = mVertexCode.size();
    vertexModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(mVertexCode.data());
    if (vkCreateShaderModule(StateBoard::device->GetLogical(), &vertexModuleCreateInfo, nullptr, &mVertexModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex shader module!");
    }

    mFragmentCode = GetSpvBinary(fragmentSpvPath);
    VkShaderModuleCreateInfo fragmentModuleCreateInfo = {};
    fragmentModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentModuleCreateInfo.pNext = nullptr;
    fragmentModuleCreateInfo.flags = 0;
    fragmentModuleCreateInfo.codeSize = mFragmentCode.size();
    fragmentModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(mFragmentCode.data());
    if (vkCreateShaderModule(StateBoard::device->GetLogical(), &fragmentModuleCreateInfo, nullptr, &mFragmentModule) != VK_SUCCESS)
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

    mFragmentShaderStageInfo = {};
    mFragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    mFragmentShaderStageInfo.pNext = nullptr;
    mFragmentShaderStageInfo.flags = 0;
    mFragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    mFragmentShaderStageInfo.module = mFragmentModule;
    mFragmentShaderStageInfo.pName = "main";
    mFragmentShaderStageInfo.pSpecializationInfo = nullptr;
}

void PipelineCore::InitDynamicStateInfo()
{
    mDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    mDynamicStateInfo.pNext = nullptr;
    mDynamicStateInfo.flags = 0;
    mDynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(mDynamicStates.size());
    mDynamicStateInfo.pDynamicStates = mDynamicStates.data();
}

void PipelineCore::InitVertexInputStateInfo
(
    const std::vector<VkVertexInputBindingDescription>& bindingDesciptions,
    const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions
)
{
    mVertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    mVertexInputStateInfo.pNext = nullptr;
    mVertexInputStateInfo.vertexBindingDescriptionCount = 1;
    mVertexInputStateInfo.pVertexBindingDescriptions = bindingDesciptions.data();
    mVertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    mVertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
}

void PipelineCore::InitInputAssemblyStateInfo()
{
    mInputAssembly = {};
    mInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    mInputAssembly.pNext = nullptr;
    mInputAssembly.flags = 0;
    mInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    mInputAssembly.primitiveRestartEnable = VK_FALSE;
}

void PipelineCore::InitViewportStateInfo()
{
    mScissor = {};
    mScissor.offset = { 0, 0 };
    mScissor.extent = StateBoard::swapchain->GetExtent();

    mViewportState = {};
    mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    mViewportState.pNext = nullptr;
    mViewportState.flags = 0;
    mViewportState.viewportCount = 1;
    mViewportState.scissorCount = 1;
    mViewportState.pScissors = &mScissor;
}

void PipelineCore::InitRasterizerInfo()
{
    mRasterizer = {};
    mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    mRasterizer.depthClampEnable = VK_FALSE;
    mRasterizer.rasterizerDiscardEnable = VK_FALSE;
    mRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    mRasterizer.lineWidth = 1.0f;
    mRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    mRasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    mRasterizer.depthBiasEnable = VK_FALSE;
    mRasterizer.depthBiasConstantFactor = 0.0f;
    mRasterizer.depthBiasClamp = 0.0f;
    mRasterizer.depthBiasSlopeFactor = 0.0f;
}

void PipelineCore::InitMultisamplingInfo()
{
    mMultisampling = {};
    mMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    mMultisampling.sampleShadingEnable = VK_FALSE;
    mMultisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
}

void PipelineCore::InitColorBlendingInfo()
{
    mColorBlendAttachment = {};
    mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    mColorBlendAttachment.blendEnable = VK_FALSE;

    mColorBlending = {};
    mColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    mColorBlending.logicOpEnable = VK_FALSE;
    mColorBlending.logicOp = VK_LOGIC_OP_COPY;
    mColorBlending.attachmentCount = 1;
    mColorBlending.pAttachments = &mColorBlendAttachment;
    mColorBlending.blendConstants[0] = 0.0f;
    mColorBlending.blendConstants[1] = 0.0f;
    mColorBlending.blendConstants[2] = 0.0f;
    mColorBlending.blendConstants[3] = 0.0f;
}

void PipelineCore::InitDescriptorSetLayoutInfo
(
    const std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings 
)
{
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = descriptorBindings.size();
    layoutInfo.pBindings = descriptorBindings.data();

    if (vkCreateDescriptorSetLayout(StateBoard::device->GetLogical(), &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void PipelineCore::InitPipelineLayoutInfo()
{
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

void PipelineCore::InitRenderPassInfo()
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

void PipelineCore::InitPipelineInfo()
{
    mGraphicsPipelineInfo = {};
    mGraphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    mGraphicsPipelineInfo.stageCount = 2;
    mGraphicsPipelineInfo.pStages = &mVertexShaderStageInfo;
    mGraphicsPipelineInfo.pVertexInputState = &mVertexInputStateInfo;
    mGraphicsPipelineInfo.pInputAssemblyState = &mInputAssembly;
    mGraphicsPipelineInfo.pViewportState = &mViewportState;
    mGraphicsPipelineInfo.pRasterizationState = &mRasterizer;
    mGraphicsPipelineInfo.pMultisampleState = &mMultisampling;
    mGraphicsPipelineInfo.pDepthStencilState = nullptr;
    mGraphicsPipelineInfo.pColorBlendState = &mColorBlending;
    mGraphicsPipelineInfo.pDynamicState = &mDynamicStateInfo;
    mGraphicsPipelineInfo.layout = mPipelineLayout;
    mGraphicsPipelineInfo.renderPass = mRenderPass;
    mGraphicsPipelineInfo.subpass = 0;
    mGraphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    mGraphicsPipelineInfo.basePipelineIndex = -1;
}

} // namespace tlr
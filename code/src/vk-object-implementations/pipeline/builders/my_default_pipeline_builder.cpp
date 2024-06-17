#include "my_default_pipeline_builder.hpp"

namespace tlr
{

DefaultPipelineBuilder::DefaultPipelineBuilder(const IDevice& device) : mDevice(device) { }

DefaultPipelineBuilder::~DefaultPipelineBuilder()
{
    vkDestroyPipelineLayout(mDevice.GetLogical(), mPipelineLayout, nullptr);
    vkDestroyRenderPass(mDevice.GetLogical(), mRenderPass, nullptr);
    vkDestroyShaderModule(mDevice.GetLogical(), mVertexModule, nullptr);
    vkDestroyShaderModule(mDevice.GetLogical(), mFragmentModule, nullptr);
}

VkGraphicsPipelineCreateInfo DefaultPipelineBuilder::GetCreateInfo()
{
    return mGraphicsPipeline;
}

VkRenderPass DefaultPipelineBuilder::GetRenderPass()
{
    return mRenderPass;
}

void DefaultPipelineBuilder::SetVertexVkShaderModuleCreateInfo(const std::string& spvFilePath)
{
    mVertexCode = GetSpvBinary(spvFilePath);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = mVertexCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(mVertexCode.data());
    mVertexShaderModuleInfo = createInfo;

    if (vkCreateShaderModule(mDevice.GetLogical(), &createInfo, nullptr, &mVertexModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vertex shader module!");
    }
}

void DefaultPipelineBuilder::SetFragmentVkShaderModuleCreateInfo(const std::string& spvFilePath)
{
    mFragmentCode = GetSpvBinary(spvFilePath);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = mFragmentCode.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(mFragmentCode.data());
    mFragmentShaderModuleInfo = createInfo;

    if (vkCreateShaderModule(mDevice.GetLogical(), &createInfo, nullptr, &mFragmentModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create fragment shader module!");
    }
}

void DefaultPipelineBuilder::SetVertexVkPipelineShaderStageCreateInfo()
{
    VkPipelineShaderStageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    createInfo.module = mVertexModule;
    createInfo.pName = "main";
    createInfo.pSpecializationInfo = nullptr;
    mVertexShaderStageInfo = createInfo;
}

void DefaultPipelineBuilder::SetFragmentVkPipelineShaderStageCreateInfo()
{
    VkPipelineShaderStageCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    createInfo.module = mFragmentModule;
    createInfo.pName = "main";
    createInfo.pSpecializationInfo = nullptr;
    mFragmentShaderStageInfo = createInfo;
}

void DefaultPipelineBuilder::SetVkPipelineDynamicStateCreateInfo()
{
    VkPipelineDynamicStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.dynamicStateCount = static_cast<uint32_t>(mDynamicStates.size());
    createInfo.pDynamicStates = mDynamicStates.data();
    mDynamicStateInfo = createInfo;
}

void DefaultPipelineBuilder::SetVkPipelineVertexInputStateCreateInfo()
{
    mBindingDescription = Vertex::GetBindingDescription();
    mAttributeDescription = Vertex::GetAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.vertexBindingDescriptionCount = 1;
    createInfo.pVertexBindingDescriptions = &mBindingDescription;
    createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(mAttributeDescription.size());
    createInfo.pVertexAttributeDescriptions = mAttributeDescription.data();
    mVertexInputStateInfo = createInfo;
}

void DefaultPipelineBuilder::SetVkPipelineInputAssemblyStateCreateInfo()
{
    VkPipelineInputAssemblyStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    createInfo.primitiveRestartEnable = VK_FALSE;
    mInputAssembly = createInfo;
}

void DefaultPipelineBuilder::SetVkPipelineViewportStateCreateInfo(const MySwapchain& swapchain)
{
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.GetExtent();
    mScissor = scissor;

    VkPipelineViewportStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.viewportCount = 1;
    createInfo.scissorCount = 1;
    createInfo.pScissors = &mScissor;
    mViewportState = createInfo;
}

void DefaultPipelineBuilder::SetRasterizerVkPipelineRasterizationStateCreateInfo()
{
    VkPipelineRasterizationStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createInfo.depthClampEnable = VK_FALSE;
    createInfo.rasterizerDiscardEnable = VK_FALSE;
    createInfo.polygonMode = VK_POLYGON_MODE_FILL;
    createInfo.lineWidth = 1.0f;
    createInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    createInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    createInfo.depthBiasEnable = VK_FALSE;
    createInfo.depthBiasConstantFactor = 0.0f;
    createInfo.depthBiasClamp = 0.0f;
    createInfo.depthBiasSlopeFactor = 0.0f;
    mRasterizer = createInfo;   
}

void DefaultPipelineBuilder::SetVkPipelineMultisampleStateCreateInfo()
{
    VkPipelineMultisampleStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    createInfo.sampleShadingEnable = VK_FALSE;
    createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    mMultisampling = createInfo;
}

void DefaultPipelineBuilder::SetVkPipelineColorBlendStateCreateInfo()
{
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    mColorBlendAttachment = colorBlendAttachment;

    VkPipelineColorBlendStateCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    createInfo.logicOpEnable = VK_FALSE;
    createInfo.logicOp = VK_LOGIC_OP_COPY;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = &mColorBlendAttachment;
    createInfo.blendConstants[0] = 0.0f;
    createInfo.blendConstants[1] = 0.0f;
    createInfo.blendConstants[2] = 0.0f;
    createInfo.blendConstants[3] = 0.0f;
    mColorBlending = createInfo;
}

void DefaultPipelineBuilder::SetVkPipelineLayoutCreateInfo()
{
    VkPipelineLayoutCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 0;
    createInfo.pSetLayouts = nullptr;
    createInfo.pushConstantRangeCount = 0;
    createInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(mDevice.GetLogical(), &createInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    mPipelineLayoutInfo = createInfo;
}

void DefaultPipelineBuilder::SetVkRenderPassCreateInfo(const MySwapchain& swapchain)
{
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapchain.GetFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    mColorAttachments = &colorAttachment;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    mReferences = &colorAttachmentRef;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    mSubpasses = &subpass;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    mDependencies = &dependency;

    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = mColorAttachments;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = mSubpasses;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = mDependencies;

    if (vkCreateRenderPass(mDevice.GetLogical(), &createInfo, nullptr, &mRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }

    mRenderPassInfo = createInfo;
}

void DefaultPipelineBuilder::SetVkGraphicsPipelineCreateInfo()
{
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = &mVertexShaderStageInfo;
    pipelineInfo.pVertexInputState = &mVertexInputStateInfo;
    pipelineInfo.pInputAssemblyState = &mInputAssembly;
    pipelineInfo.pViewportState = &mViewportState;
    pipelineInfo.pRasterizationState = &mRasterizer;
    pipelineInfo.pMultisampleState = &mMultisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &mColorBlending;
    pipelineInfo.pDynamicState = &mDynamicStateInfo;
    pipelineInfo.layout = mPipelineLayout;
    pipelineInfo.renderPass = mRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;
    mGraphicsPipeline = pipelineInfo;
}

const VkGraphicsPipelineCreateInfo& DefaultPipelineBuilder::Build(const std::string& vertexSpv, const std::string& fragmentSpv, const MySwapchain& swapchain)
{
    SetVertexVkShaderModuleCreateInfo(vertexSpv);
    SetFragmentVkShaderModuleCreateInfo(fragmentSpv);
    SetVertexVkPipelineShaderStageCreateInfo();
    SetFragmentVkPipelineShaderStageCreateInfo();
    SetVkPipelineDynamicStateCreateInfo();
    SetVkPipelineVertexInputStateCreateInfo();
    SetVkPipelineInputAssemblyStateCreateInfo();
    SetVkPipelineViewportStateCreateInfo(swapchain);
    SetRasterizerVkPipelineRasterizationStateCreateInfo();
    SetVkPipelineMultisampleStateCreateInfo();
    SetVkPipelineColorBlendStateCreateInfo();
    SetVkPipelineLayoutCreateInfo();
    SetVkRenderPassCreateInfo(swapchain);
    SetVkGraphicsPipelineCreateInfo();
    return mGraphicsPipeline;
}

std::vector<char> DefaultPipelineBuilder::GetSpvBinary(const std::string& fileName)
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
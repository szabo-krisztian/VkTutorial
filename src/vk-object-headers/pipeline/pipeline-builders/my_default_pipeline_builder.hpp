#pragma once

#include "ipipeline_builder.h"
#include "idevice.h"

#include <fstream>
#include <iostream>

namespace tlr
{

class DefaultPipelineBuilder : public IPipelineBuilder
{
public:
    DefaultPipelineBuilder(const IDevice& device);
    ~DefaultPipelineBuilder() override;
    
    VkGraphicsPipelineCreateInfo GetCreateInfo();
    VkRenderPass GetRenderPass();

    virtual void SetVertexVkShaderModuleCreateInfo(const std::string& spvFilePath)   override;
    virtual void SetFragmentVkShaderModuleCreateInfo(const std::string& spvFilePath) override;
    virtual void SetVertexVkPipelineShaderStageCreateInfo()                          override;
    virtual void SetFragmentVkPipelineShaderStageCreateInfo()                        override;
    virtual void SetVkPipelineDynamicStateCreateInfo()                               override;
    virtual void SetVkPipelineVertexInputStateCreateInfo()                           override;
    virtual void SetVkPipelineInputAssemblyStateCreateInfo()                         override;
    virtual void SetVkPipelineViewportStateCreateInfo(const MySwapchain& swapchain)  override;
    virtual void SetRasterizerVkPipelineRasterizationStateCreateInfo()               override;
    virtual void SetVkPipelineMultisampleStateCreateInfo()                           override;
    virtual void SetVkPipelineColorBlendStateCreateInfo()                            override;
    virtual void SetVkPipelineLayoutCreateInfo()                                     override;
    virtual void SetVkRenderPassCreateInfo(const MySwapchain& swapchain)             override;
    virtual void SetVkGraphicsPipelineCreateInfo()                                   override;

    virtual const VkGraphicsPipelineCreateInfo& Build
    (
        const std::string& vertexSpv,
        const std::string& fragmentSpv,
        const MySwapchain& swapchain
    ) override;

private:
    const IDevice&                         mDevice;
    std::vector<char>                      mVertexCode;
    std::vector<char>                      mFragmentCode;
    VkShaderModule                         mVertexModule;
    VkShaderModule                         mFragmentModule;
    const std::vector<VkDynamicState>      mDynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkRect2D                               mScissor;
    VkPipelineColorBlendAttachmentState    mColorBlendAttachment;
    VkPipelineLayout                       mPipelineLayout;
    VkAttachmentDescription*               mColorAttachments;
    VkAttachmentReference*                 mReferences;
    VkSubpassDescription*                  mSubpasses;
    VkSubpassDependency*                   mDependencies;
    VkRenderPass                           mRenderPass;

    VkShaderModuleCreateInfo               mVertexShaderModuleInfo;
    VkShaderModuleCreateInfo               mFragmentShaderModuleInfo;
    VkPipelineShaderStageCreateInfo        mVertexShaderStageInfo;
    VkPipelineShaderStageCreateInfo        mFragmentShaderStageInfo;
    VkPipelineDynamicStateCreateInfo       mDynamicStateInfo;
    VkPipelineVertexInputStateCreateInfo   mVertexInputStateInfo;
    VkPipelineInputAssemblyStateCreateInfo mInputAssembly;
    VkPipelineViewportStateCreateInfo      mViewportState;
    VkPipelineRasterizationStateCreateInfo mRasterizer;
    VkPipelineMultisampleStateCreateInfo   mMultisampling;
    VkPipelineColorBlendStateCreateInfo    mColorBlending;
    VkPipelineLayoutCreateInfo             mPipelineLayoutInfo;
    VkRenderPassCreateInfo                 mRenderPassInfo;
    VkGraphicsPipelineCreateInfo           mGraphicsPipeline;

    static std::vector<char> GetSpvBinary(const std::string& fileName);
};

} // namespace tlr
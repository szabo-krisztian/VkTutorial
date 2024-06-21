#pragma once

#include "state_board.hpp"

#include <fstream>
#include <iostream>

namespace tlr
{

class PipelineCore
{
public:
    PipelineCore
    (
        const std::string&                                    vertexSpvPath,
        const std::string&                                    fragmentSpvPath,
        const std::vector<VkVertexInputBindingDescription>&   bindingDesciptions,
        const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
        const std::vector<VkDescriptorSetLayoutBinding>&      descriptorBindings
    );
    ~PipelineCore();

protected:
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
    VkPipelineLayoutCreateInfo             mPipelineLayoutInfo;
    VkRenderPassCreateInfo                 mRenderPassInfo;
    VkGraphicsPipelineCreateInfo           mGraphicsPipelineInfo;

    void                                   InitVertexInputStateInfo
    (
        const std::vector<VkVertexInputBindingDescription>& bindingDesciptions,
        const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions
    );
    void                                   InitDescriptorSetLayoutInfo
    (
        const std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings 
    );


private:
    std::vector<char>                      mVertexCode; //?
    std::vector<char>                      mFragmentCode; //?
    VkShaderModule                         mVertexModule;
    VkShaderModule                         mFragmentModule;
    const std::vector<VkDynamicState>      mDynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkRect2D                               mScissor; //?
    VkPipelineColorBlendAttachmentState    mColorBlendAttachment; //?
    VkDescriptorSetLayout                  mDescriptorSetLayout;
    VkPipelineLayout                       mPipelineLayout;
    VkRenderPass                           mRenderPass;

    static std::vector<char>               GetSpvBinary(const std::string& fileName);
    void                                   InitShaderInfos(const std::string& vertexSpvPath, const std::string& fragmentSpvPath);
    void                                   InitDynamicStateInfo();
    void                                   InitInputAssemblyStateInfo();
    void                                   InitViewportStateInfo();
    void                                   InitRasterizerInfo();
    void                                   InitMultisamplingInfo();
    void                                   InitColorBlendingInfo();
    void                                   InitPipelineLayoutInfo();
    void                                   InitRenderPassInfo();
    void                                   InitPipelineInfo();
};

} // namespace tlr
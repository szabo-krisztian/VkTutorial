#pragma once

#include "state_board.hpp"

#include <fstream>
#include <iostream>

namespace tlr
{

class PipelineBlueprint
{
public:
    PipelineBlueprint();
    ~PipelineBlueprint();

    void SetVertexShaderSpvPath(const std::string& vertexSpvPath);
    void SetFragmentShaderSpvPath(const std::string& fragmentSpvPath);
    void SetVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindingDesciptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);
    void SetLayout(const std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings);

private:
    VkPipelineShaderStageCreateInfo        mVertexShaderStageInfo;
    VkPipelineShaderStageCreateInfo        mFragmentShaderStageInfo;
    VkPipelineDynamicStateCreateInfo       mDynamicStateInfo;
    VkPipelineVertexInputStateCreateInfo   mVertexInputStateInfo;
    VkPipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
    VkPipelineViewportStateCreateInfo      mViewportStateInfo;
    VkPipelineRasterizationStateCreateInfo mRasterizerInfo;
    VkPipelineMultisampleStateCreateInfo   mMultisamplingInfo;
    VkPipelineColorBlendStateCreateInfo    mColorBlendingInfo;
    VkPipelineLayoutCreateInfo             mPipelineLayoutInfo;
    VkPipelineLayoutCreateInfo             mPipelineLayoutInfo;
    VkRenderPassCreateInfo                 mRenderPassInfo;
    VkGraphicsPipelineCreateInfo           mGraphicsPipelineInfo;

    VkShaderModule                         mVertexModule;
    VkShaderModule                         mFragmentModule;
    VkRect2D                               mScissor;
    VkPipelineColorBlendAttachmentState    mColorBlendAttachment;
    VkDescriptorSetLayout                  mDescriptorSetLayout;
    VkPipelineLayout                       mPipelineLayout;
    VkRenderPass                           mRenderPass;
    VkPipeline                             mPipeline;
    
    void                                   InitDynamicStateInfo();
    void                                   InitInputAssemblyStateInfo();
    void                                   InitViewportStateInfo();
    void                                   InitRasterizerInfo();
    void                                   InitMultisamplingInfo();
    void                                   InitColorBlendingInfo();
    void                                   InitRenderPassInfo();
    void                                   BuildPipelineCreateInfo();
    std::vector<char>                      GetSpvBinary(const std::string& fileName);
};

} // namespace tlr
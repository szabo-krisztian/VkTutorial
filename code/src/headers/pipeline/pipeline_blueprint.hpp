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

    const VkPipeline&                      Get()                 const;
    const VkPipelineLayout&                GetLayout()           const;
    const VkRenderPass&                    GetRenderPass()       const;
    const VkDescriptorSetLayout&           GetDescriptorLayout() const;
    
protected:
    void                                   SetVertexShaderSpvPath(const std::string& vertexSpvPath);
    void                                   SetFragmentShaderSpvPath(const std::string& fragmentSpvPath);
    void                                   SetVertexInputState(uint32_t bindingCount, const VkVertexInputBindingDescription* bindingDesciptions, uint32_t attributeCount, const VkVertexInputAttributeDescription* attributeDescriptions);
    void                                   SetLayout(); // TODO
    void                                   BuildGraphicsPipeline();

private:
    VkPipelineShaderStageCreateInfo        mVertexShaderStageInfo;
    VkPipelineShaderStageCreateInfo        mFragmentShaderStageInfo; // TODO memory allocation trick
    VkPipelineDynamicStateCreateInfo       mDynamicStateInfo;
    VkPipelineVertexInputStateCreateInfo   mVertexInputStateInfo;
    VkPipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
    VkPipelineViewportStateCreateInfo      mViewportStateInfo;
    VkPipelineRasterizationStateCreateInfo mRasterizerInfo;
    VkPipelineMultisampleStateCreateInfo   mMultisamplingInfo;
    VkPipelineColorBlendStateCreateInfo    mColorBlendingInfo;
    VkPipelineLayoutCreateInfo             mPipelineLayoutInfo;
    VkRenderPassCreateInfo                 mRenderPassInfo;
    VkGraphicsPipelineCreateInfo           mGraphicsPipelineInfo;

    const std::vector<VkDynamicState>      M_DYNAMIC_STATES = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
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
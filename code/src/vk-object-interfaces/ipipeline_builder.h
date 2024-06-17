#pragma once

#include <vulkan/vulkan.h>

#include "my_swapchain.hpp"

#include <string>
#include <vector>
#include <stdexcept>

namespace tlr
{

class IPipelineBuilder
{
public:
    virtual ~IPipelineBuilder() = default;
    
    virtual void SetVertexVkShaderModuleCreateInfo(const std::string& spvFilePath)   = 0;
    virtual void SetFragmentVkShaderModuleCreateInfo(const std::string& spvFilePath) = 0;
    virtual void SetVertexVkPipelineShaderStageCreateInfo()                          = 0;
    virtual void SetFragmentVkPipelineShaderStageCreateInfo()                        = 0;
    virtual void SetVkPipelineDynamicStateCreateInfo()                               = 0;
    virtual void SetVkPipelineVertexInputStateCreateInfo()                           = 0;
    virtual void SetVkPipelineInputAssemblyStateCreateInfo()                         = 0;
    virtual void SetVkPipelineViewportStateCreateInfo(const MySwapchain& swapchain)  = 0;
    virtual void SetRasterizerVkPipelineRasterizationStateCreateInfo()               = 0;
    virtual void SetVkPipelineMultisampleStateCreateInfo()                           = 0;
    virtual void SetVkPipelineColorBlendStateCreateInfo()                            = 0;
    virtual void SetVkPipelineLayoutCreateInfo()                                     = 0;
    virtual void SetVkRenderPassCreateInfo(const MySwapchain& swapchain)             = 0;
    virtual void SetVkGraphicsPipelineCreateInfo()                                   = 0;
    
    virtual const VkGraphicsPipelineCreateInfo& Build
    (
        const std::string& vertexSpv,
        const std::string& fragmentSpv,
        const MySwapchain& swapchain
    ) = 0;
};

} // namespace tlr
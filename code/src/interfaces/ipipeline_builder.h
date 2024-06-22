#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace tlr
{

class IPipelineBuilder
{
public:
    virtual ~IPipelineBuilder() = 0;

    virtual void BuildVertexShaderSpvPath(const std::string& vertexSpvPath) = 0;
    virtual void BuildFragmentShaderSpvPath(const std::string& fragmentSpvPath) = 0;
    virtual void BuildVertexInputState(const std::vector<VkVertexInputBindingDescription>& bindingDesciptions, const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions) = 0;
    virtual void BuildLayout(const std::vector<VkDescriptorSetLayoutBinding>& descriptorBindings) = 0;
};

} // namespace tlr
#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace tlr
{

class ShaderModule
{
public:
    ShaderModule(VkDevice& device, const std::string& spvPath, VkShaderStageFlagBits stage);
    ~ShaderModule();

    VkPipelineShaderStageCreateInfo GetCreateInfo();

private:
    VkDevice& _device;
    VkShaderStageFlagBits _stage;
    VkShaderModule _shaderModule;
};

} // namespace tlr

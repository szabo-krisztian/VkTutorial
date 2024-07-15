#pragma once

#include <vulkan/vulkan.h>
#include <vector>
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

    VkShaderModule CreateShaderModule(const std::vector<char>& code);
};

} // namespace tlr

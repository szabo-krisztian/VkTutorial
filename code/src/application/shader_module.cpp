#include "shader_module.hpp"
#include "toolset.hpp"
#include "initializers.hpp"

namespace tlr
{

ShaderModule::ShaderModule(VkDevice& device, const std::string& spvPath, VkShaderStageFlagBits stage) : _device(device), _stage(stage)
{
    std::vector<char> shaderCode = tools::ReadFile(spvPath);
    _shaderModule = tools::CreateShaderModule(_device, shaderCode);
}

ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(_device, _shaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo ShaderModule::GetCreateInfo()
{
    return init::PipelineShaderStageCreateInfo(_stage, _shaderModule);
}

} // namespace tlr
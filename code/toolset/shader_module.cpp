#include "shader_module.hpp"
#include "toolset.hpp"
#include "initializers.hpp"

namespace tlr
{

ShaderModule::ShaderModule(VkDevice& device, const std::string& spvPath, VkShaderStageFlagBits stage) : _device(device), _stage(stage)
{
    std::vector<char> shaderCode = tools::ReadFile(spvPath);
    _shaderModule = CreateShaderModule(shaderCode);
}

ShaderModule::~ShaderModule()
{
    vkDestroyShaderModule(_device, _shaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo ShaderModule::GetCreateInfo()
{
    return init::PipelineShaderStageCreateInfo(_stage, _shaderModule);
}

VkShaderModule ShaderModule::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    VK_CHECK_RESULT(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

} // namespace tlr
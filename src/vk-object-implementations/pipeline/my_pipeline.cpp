#include "my_pipeline.hpp"

namespace tlr
{

MyPipeline::MyPipeline(const VkGraphicsPipelineCreateInfo& createInfo, IDevice& device) : mDevice(device)
{
    if (vkCreateGraphicsPipelines(mDevice.GetLogical(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &mGraphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

MyPipeline::~MyPipeline()
{
    vkDestroyPipeline(mDevice.GetLogical(), mGraphicsPipeline, nullptr);
}

const VkPipeline& MyPipeline::Get()
{
    return mGraphicsPipeline;
}

} // namespace tlr
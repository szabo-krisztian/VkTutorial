#pragma once

#include "idevice.h"
#include "ipipeline_builder.h"

namespace tlr
{

class MyPipeline
{
public:
    MyPipeline(const VkGraphicsPipelineCreateInfo& createInfo, IDevice& device);
    ~MyPipeline();
    
    MyPipeline(const MyPipeline&) = delete;
    MyPipeline operator=(const MyPipeline&) = delete;

    const VkPipeline& Get();

private:
    IDevice& mDevice;
    VkPipeline mGraphicsPipeline;
};

} // namespace tlr
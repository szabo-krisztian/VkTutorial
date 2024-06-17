#pragma once

#include <vulkan/vulkan.h>

namespace tlr
{

class IInstanceBuilder
{
public:
    virtual ~IInstanceBuilder() = default;

    virtual void                        SetApplicationCreateInfo() = 0;
    virtual void                        SetVkInstanceCreateInfo()  = 0;
    virtual const VkInstanceCreateInfo& Build()                    = 0;
};

} // namespace tlr
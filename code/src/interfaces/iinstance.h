#pragma once

#include <vulkan/vulkan.h>

namespace tlr
{

class IInstance
{
public:
    virtual ~IInstance() = default;
    
    virtual const VkInstance& Get() const = 0;
};

} // namespace tlr
#pragma once

#include "iinstance_builder.h"

#include <vulkan/vulkan.h>
#include <iostream>

namespace tlr
{

class DefaultInstanceBuilder : public IInstanceBuilder 
{
public:
    DefaultInstanceBuilder();
    virtual ~DefaultInstanceBuilder();

    virtual void                        SetApplicationCreateInfo() override;
    virtual void                        SetVkInstanceCreateInfo()  override;
    virtual const VkInstanceCreateInfo& Build()                    override;

protected:
    VkInstanceCreateInfo                mInstanceCreateInfo;
    VkApplicationInfo                   mApplicationCreateInfo;
};

} // namespace tlr
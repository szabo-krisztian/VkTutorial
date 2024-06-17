#pragma once

#include "iinstance_builder.h"

#include <stdexcept>

namespace tlr
{

class MyInstance
{
public:
    MyInstance(IInstanceBuilder& builder);
    ~MyInstance();

    MyInstance(const MyInstance&) = delete;
    MyInstance operator=(const MyInstance&) = delete;

    const VkInstance& Get() const;
    
private:
    IInstanceBuilder& mBuilder;
    VkInstance        mInstance;
};

} // namespace tlr
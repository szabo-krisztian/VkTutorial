#include "my_instance.hpp"

namespace tlr
{

MyInstance::MyInstance(IInstanceBuilder& builder) : mBuilder(builder)
{
    if (vkCreateInstance(&builder.Build(), nullptr, &mInstance))
    {
        throw std::runtime_error("instance creation failure!");
    }
}

MyInstance::~MyInstance()
{
    vkDestroyInstance(mInstance, nullptr);
}

const VkInstance& MyInstance::Get() const
{
    return mInstance;
} 

} // namespace tlr
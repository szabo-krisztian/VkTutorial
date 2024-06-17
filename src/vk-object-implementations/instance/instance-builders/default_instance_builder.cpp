#include "default_instance_builder.hpp"

namespace tlr
{

DefaultInstanceBuilder::DefaultInstanceBuilder() { }

DefaultInstanceBuilder::~DefaultInstanceBuilder() { }

void DefaultInstanceBuilder::SetApplicationCreateInfo()
{
    mApplicationCreateInfo = {};
    mApplicationCreateInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    mApplicationCreateInfo.pNext = nullptr;
    mApplicationCreateInfo.pApplicationName = "Vulkan Application";
    mApplicationCreateInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    mApplicationCreateInfo.pEngineName = "No Engine";
    mApplicationCreateInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    mApplicationCreateInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);
}

void DefaultInstanceBuilder::SetVkInstanceCreateInfo()
{
    mInstanceCreateInfo = {};
    mInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    mInstanceCreateInfo.pNext = nullptr;
    mInstanceCreateInfo.flags = 0;
    mInstanceCreateInfo.pApplicationInfo = &mApplicationCreateInfo;
    mInstanceCreateInfo.enabledLayerCount = 0;
    mInstanceCreateInfo.ppEnabledLayerNames = nullptr;
    mInstanceCreateInfo.enabledExtensionCount = 0;
    mInstanceCreateInfo.ppEnabledExtensionNames = nullptr;
}

const VkInstanceCreateInfo& DefaultInstanceBuilder::Build()
{
    SetApplicationCreateInfo();
    SetVkInstanceCreateInfo();
    return mInstanceCreateInfo;
}

} // namespace tlr
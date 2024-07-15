#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace tlr
{

struct Instance
{
    VkInstance               instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    operator VkInstance() const
    {
        return instance;
    }
};

} // namespace tlr
#include "app_base.hpp"

namespace tlr
{

AppBase::~AppBase()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    DestroyDebugMessenger(instance, debugMessenger);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void AppBase::Init()
{
    glfwInit();
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
    InitVulkan();

    _isInitizalized = true;
}

void AppBase::InitVulkan()
{
    InstanceBuilder builder;
    builder.SetApplicationName("Vulkan app")
           .SetApplicationVersion(VK_MAKE_VERSION(1, 0,0 ))
           .SetEngineName("No engine")
           .SetEngineVersion(VK_MAKE_VERSION(1, 0, 0))
           .SetApiVersion(VK_MAKE_VERSION(1, 1, 0))
           .RequestDefaultLayers()
           .RequestDefaultExtensions()
           .UseDebugMessenger()
           .Build();
    instance = builder.GetInstance();
    debugMessenger = builder.GetMessengerInstance();

    VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface), "failed to create window surface!");

    PhysicalDeviceSelector physicalDeviceSelector{instance, surface};
    physicalDevice = physicalDeviceSelector.EnableDedicatedGPU()
                                           .Select();

    DeviceBuilder deviceBuilder{physicalDevice};
    device = deviceBuilder.EnableValidationLayers().Build();
}

} // namespace tlr
#include "app_base.hpp"

#include "toolset.hpp"
#include "instance_builder.hpp"
#include "physical_device_selector.hpp"
#include "device_builder.hpp"
#include "swapchain_builder.hpp"

namespace tlr
{

AppBase::AppBase()
{
    Init();
}

AppBase::~AppBase()
{
    for (auto imageView : swapchain.imageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    DestroyDebugMessenger(instance, debugMessenger);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void AppBase::Init()
{
    InitGLFW();
    InitVulkan();
    InitSwapchain();

    _isInitizalized = true;
}

void AppBase::InitGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", nullptr, nullptr);
}

void AppBase::InitVulkan()
{
    InstanceBuilder builder;
    Instance instances = builder.SetApplicationName("Vulkan app")
                                .SetApplicationVersion(VK_MAKE_VERSION(1, 0,0 ))
                                .SetEngineName("No engine")
                                .SetEngineVersion(VK_MAKE_VERSION(1, 0, 0))
                                .SetApiVersion(VK_MAKE_VERSION(1, 1, 0))
                                .RequestDefaultLayers()
                                .RequestDefaultExtensions()
                                .UseDebugMessenger()
                                .Build();
    instance = instances.instance;
    debugMessenger = instances.debugMessenger;

    VK_CHECK_RESULT(glfwCreateWindowSurface(instance, window, nullptr, &surface));
    
    PhysicalDeviceSelector physicalDeviceSelector{instance, surface};
    physicalDevice = physicalDeviceSelector.EnableDedicatedGPU()
                                           .Select();
    
    DeviceBuilder deviceBuilder{physicalDevice};
    device = deviceBuilder.EnableValidationLayers()
                          .Build();
}

void AppBase::InitSwapchain()
{
    SwapchainBuilder builder(window, surface, physicalDevice, device);
    swapchain = builder.SetDesiredFormat(VK_FORMAT_B8G8R8A8_SRGB)
                       .SetDesiredColorSpace(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                       .SetDesiredPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
                       .SetDesiredExtent(WINDOW_WIDTH, WINDOW_HEIGHT)
                       .SetDesiredImageCount(physicalDevice.swapchainSupportDetails.capabilities.minImageCount + 1)
                       .SetDesiredArrayLayerCount(1)
                       .SetImageFlags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                       .Build();
}

} // namespace tlr
#pragma

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "physical_device.hpp"
#include "device.hpp"
#include "swapchain.hpp"

namespace tlr
{

class AppBase
{
public:
    AppBase();
    ~AppBase();

protected:
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    VkInstance               instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    GLFWwindow               *window;
    VkSurfaceKHR             surface;
    PhysicalDevice           physicalDevice;
    Device                   device;
    Swapchain                swapchain;

private:
    bool _isInitizalized = false;
    
    void Init();
    void InitGLFW();
    void InitVulkan();
    void InitSwapchain();
};

} // namespace tlr

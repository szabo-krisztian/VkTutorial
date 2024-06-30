#pragma

#include "bootstrap.hpp"

namespace tlr
{

class AppBase
{
public:
    ~AppBase();

    void Init();
    
protected:
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    VkInstance               instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    PhysicalDevice           physicalDevice;
    Device                   device;
    VkSurfaceKHR             surface;
    GLFWwindow*              window;

private:
    bool _isInitizalized = false;
    
    void InitVulkan();
};

} // namespace tlr

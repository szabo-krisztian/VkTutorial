#pragma

#include "bootstrap.hpp"

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
    PhysicalDevice           physicalDevice;
    Device                   device;
    VkSurfaceKHR             surface;
    GLFWwindow*              window;

    VkSwapchainKHR           swapchain;
	VkFormat                 swapchainImageFormat;
	std::vector<VkImage>     swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	VkExtent2D               swapchainExtent;

private:
    bool _isInitizalized = false;
    
    void Init();
    void InitVulkan();
};

} // namespace tlr

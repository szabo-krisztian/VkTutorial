#pragma once

#include "my_instance.hpp"
#include "my_window.hpp"

#include <optional>
#include <set>

namespace tlr
{

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

class MyVulkanDevice
{
public:
    MyVulkanDevice(MyVulkanInstance& instance, MyVulkanWindow& window);
    ~MyVulkanDevice();

    MyVulkanDevice(const MyVulkanDevice&) = delete;
    MyVulkanDevice operator=(const MyVulkanDevice&) = delete;

    const VkPhysicalDevice&        GetPhysical()           const;
    const VkDevice&                GetLogical()            const;
    const QueueFamilyIndices&      GetQueueFamilyIndices() const;
    const SwapChainSupportDetails& GetSwapChainSupport()   const;

private:
    const std::vector<const char*> M_VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> M_DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#ifdef NDEBUG
    const bool                     M_ENABLE_VALIDATION_LAYERS = false;
#else
    const bool                     M_ENABLE_VALIDATION_LAYERS = true;
#endif

    MyVulkanInstance&       mInstance;
    MyVulkanWindow&         mWindow;
    VkPhysicalDevice        mPhysicalDevice;
    QueueFamilyIndices      mFamilyIndices;
    SwapChainSupportDetails mSwapChainSupportDetails;
    VkDevice                mLogicalDevice;
    VkQueue                 mGraphicsQueue;
    VkQueue                 mPresentQueue;

    VkPhysicalDevice                     GetPhysicalDevice();
    std::vector<VkPhysicalDevice>        GetPhysicalDevices();
    bool                                 IsPhysicalDeviceSuitable(const VkPhysicalDevice& device);
    bool                                 IsDeviceExtensionsSupported(const VkPhysicalDevice& physicalDevice);
    SwapChainSupportDetails              GetSwapChainSupport(const VkPhysicalDevice& physicalDevice);
    QueueFamilyIndices                   GetQueueFamilyIndices(const VkPhysicalDevice& device);
    std::vector<VkQueueFamilyProperties> GetQueueFamilies(const VkPhysicalDevice& physicalDevice);
    VkResult                             InitLogicalDevice();
};

} // namespace tlr
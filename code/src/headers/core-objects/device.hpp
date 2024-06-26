#pragma once

#include "state_board.hpp"
#include "idevice.h"

#include <optional>
#include <set>
#include <stdexcept>

namespace tlr
{

class Device : public IDevice
{
public:
    Device();
    ~Device();

    Device(const Device&) = delete;
    Device operator=(const Device&) = delete;

    const VkPhysicalDevice&        GetPhysical()           const override;
    const VkDevice&                GetLogical()            const override;
    const VkQueue&                 GetGraphicsQueue()      const override;
    const VkQueue&                 GetPresentQueue()       const override;
    const QueueFamilyIndices&      GetQueueFamilyIndices() const override;
    const SwapchainSupportDetails& GetSwapchainSupport()   const override;

private:
    const std::vector<const char*>       M_VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*>       M_DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#ifdef NDEBUG
    const bool                           M_ENABLE_VALIDATION_LAYERS = false;
#else
    const bool                           M_ENABLE_VALIDATION_LAYERS = true;
#endif
    VkPhysicalDevice                     mPhysicalDevice;
    QueueFamilyIndices                   mFamilyIndices;
    SwapchainSupportDetails              mSwapchainSupportDetails;
    VkDevice                             mLogicalDevice;
    VkQueue                              mGraphicsQueue;
    VkQueue                              mPresentQueue;

    VkPhysicalDevice                     GetPhysicalDevice();
    std::vector<VkPhysicalDevice>        GetPhysicalDevices();
    bool                                 IsPhysicalDeviceSuitable(const VkPhysicalDevice& device);
    bool                                 IsDeviceExtensionsSupported(const VkPhysicalDevice& physicalDevice);
    SwapchainSupportDetails              GetSwapchainSupport(const VkPhysicalDevice& physicalDevice);
    QueueFamilyIndices                   GetQueueFamilyIndices(const VkPhysicalDevice& device);
    std::vector<VkQueueFamilyProperties> GetQueueFamilies(const VkPhysicalDevice& physicalDevice);
    VkResult                             InitLogicalDevice();
};

} // namespace tlr
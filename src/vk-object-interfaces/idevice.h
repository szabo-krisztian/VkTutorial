#pragma once

#include "queue_family_indices.hpp"
#include "swapchain_support_details.hpp"

#include <vulkan/vulkan.h>

namespace tlr
{

class IDevice
{
public:
    virtual ~IDevice() = default;

    virtual const VkPhysicalDevice&        GetPhysical()           const = 0;
    virtual const VkDevice&                GetLogical()            const = 0;
    virtual const VkQueue&                 GetGraphicsQueue()      const = 0;
    virtual const VkQueue&                 GetPresentQueue()       const = 0;
    virtual const QueueFamilyIndices&      GetQueueFamilyIndices() const = 0;
    virtual const SwapchainSupportDetails& GetSwapchainSupport()   const = 0;
};

} // namespace tlr
#pragma once

#include "queue_family_indices.hpp"
#include "swap_chain_support_details.hpp"

#include <vulkan/vulkan.h>

namespace tlr
{

class IVulkanDevice
{
public:
    virtual ~IVulkanDevice() = default;

    virtual const VkPhysicalDevice&        GetPhysical()           const = 0;
    virtual const VkDevice&                GetLogical()            const = 0;
    virtual const VkQueue&                 GetGraphicsQueue()      const = 0;
    virtual const VkQueue&                 GetPresentQueue()       const = 0;
    virtual const QueueFamilyIndices&      GetQueueFamilyIndices() const = 0;
    virtual const SwapChainSupportDetails& GetSwapChainSupport()   const = 0;
};

} // namespace tlr
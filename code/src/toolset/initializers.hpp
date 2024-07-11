#pragma once

#include <vector>

#include <vulkan/vulkan.h>

namespace tlr
{

    namespace init
    {

    VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1);

    VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);

    VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

    VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

    } // namespace init

} // namespace tlr
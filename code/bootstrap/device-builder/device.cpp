/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Charles Giessen's Vulkan Code
 *   Copyright (c) 2020 Charles Giessen
 *   Licensed under the MIT License
 *
 * - Sascha Willems' Vulkan Buffer Class
 *   Copyright (c) 2016 Sascha Willems
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#include "device.hpp"

#include <cstring>

#include "toolset.hpp"
#include "initializers.hpp"

namespace tlr
{

uint32_t Device::GetMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);   
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkResult Device::CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, Buffer *buffer, VkDeviceSize size, void *data)
{
    buffer->device = device;

    VkBufferCreateInfo bufferCreateInfo = init::BufferCreateInfo(usageFlags, size);
    VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer->buffer));

    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = init::MemoryAllocateInfo();
    vkGetBufferMemoryRequirements(device, buffer->buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    VK_CHECK_RESULT(vkAllocateMemory(device, &memAlloc, nullptr, &buffer->memory));

    buffer->alignment = memReqs.alignment;
    buffer->size = size;
    buffer->usageFlags = usageFlags;
    buffer->memoryPropertyFlags = memoryPropertyFlags;

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        VK_CHECK_RESULT(buffer->Map());
        memcpy(buffer->mapped, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            buffer->Flush();

        buffer->Unmap();
    }

    // Initialize a default descriptor that covers the whole buffer size
    buffer->SetupDescriptor();

    // Attach the memory to the buffer object
    return buffer->Bind();
}

} // namespace tlr

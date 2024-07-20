/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Sascha Willems' Vulkan Buffer Class
 *   Copyright (c) 2016 Sascha Willems
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#include "buffer.hpp"

#include <cstring>

namespace tlr
{	
	/** 
	* Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
	* 
	* @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	* 
	* @return VkResult of the buffer mapping call
	*/
	VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(device, memory, offset, size, 0, &mapped);
	}

	/**
	* Unmap a mapped memory range
	*
	* @note Does not return a result as vkUnmapMemory can't fail
	*/
	void Buffer::Unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(device, memory);
			mapped = nullptr;
		}
	}

	/** 
	* Attach the allocated memory block to the buffer
	* 
	* @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
	* 
	* @return VkResult of the bindBufferMemory call
	*/
	VkResult Buffer::Bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(device, buffer, memory, offset);
	}

	/**
	* Setup the default descriptor for this buffer
	*
	* @param size (Optional) Size of the memory range of the descriptor
	* @param offset (Optional) Byte offset from beginning
	*
	*/
	void Buffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range = size;
	}

	/**
	* Copies the specified data to the mapped buffer
	* 
	* @param data Pointer to the data to copy
	* @param size Size of the data to copy in machine units
	*
	*/
	void Buffer::CopyTo(const void* data, VkDeviceSize size)
	{
		assert(mapped);
		memcpy(mapped, data, size);
	}

	/** 
	* Flush a memory range of the buffer to make it visible to the device
	*
	* @note Only required for non-coherent memory
	*
	* @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the flush call
	*/
	VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
	}

	/**
	* Invalidate a memory range of the buffer to make it visible to the host
	*
	* @note Only required for non-coherent memory
	*
	* @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the invalidate call
	*/
	VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
	}

	/** 
	* Release all Vulkan resources held by this buffer
	*/
	void Buffer::Destroy()
	{
		if (buffer)
		{
			vkDestroyBuffer(device, buffer, nullptr);
		}
		if (memory)
		{
			vkFreeMemory(device, memory, nullptr);
		}
	}
};
/*
 * This file includes code that is licensed under the MIT License.
 *
 * - Sascha Willems' Vulkan Buffer Class
 *   Copyright (c) 2016 Sascha Willems
 *   Licensed under the MIT License
 *
 * For the full text of the MIT License, see the LICENSE.md file in the root of the project.
 */

#pragma once

#include <vector>

#include <vulkan/vulkan.h>

#include "toolset.hpp"

namespace tlr
{

struct Buffer
{
	VkDevice 		       device;
	VkBuffer 			   buffer = VK_NULL_HANDLE;
	VkDeviceMemory 		   memory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo descriptor;
	VkDeviceSize 		   size = 0;
	VkDeviceSize 		   alignment = 0;
	void* 				   mapped = nullptr;
	VkBufferUsageFlags 	   usageFlags;
	VkMemoryPropertyFlags  memoryPropertyFlags;
	
	VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void 	 Unmap();
	VkResult Bind(VkDeviceSize offset = 0);
	void     SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void 	 CopyTo(const void* data, VkDeviceSize size);
	VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void 	 Destroy();
};

} // namespace tlr
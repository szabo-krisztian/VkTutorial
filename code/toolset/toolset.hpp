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

#include <assert.h>
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)																									   	   \
{																															   	   \
	VkResult res = (f);																										   	   \
	if (res != VK_SUCCESS)																									   	   \
	{																														   	   \
		std::cerr << "Fatal : VkResult is \"" << tlr::tools::ErrorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n";     \
		assert(res == VK_SUCCESS);																							   	   \
	}																														   	   \
}

namespace tlr
{

	namespace tools
	{
		
		std::string ErrorString(VkResult errorCode);

		std::vector<char> ReadFile(const std::string& filename);

	} // namespace tools

} // namespace tlr
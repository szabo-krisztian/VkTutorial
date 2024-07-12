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
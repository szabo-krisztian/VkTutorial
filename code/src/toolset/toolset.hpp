#pragma once

#include <assert.h>
#include <string>
#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)																									   	   \
{																															   	   \
	VkResult res = (f);																										   	   \
	if (res != VK_SUCCESS)																									   	   \
	{																														   	   \
		std::cerr << "Fatal : VkResult is \"" << tlr::ErrorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																							   	   \
	}																														   	   \
}


namespace tlr
{

std::string ErrorString(VkResult errorCode);

} // namespace tlr
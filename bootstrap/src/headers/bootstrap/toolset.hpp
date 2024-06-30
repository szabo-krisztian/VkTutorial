#pragma once

#include <assert.h>

#define VK_CHECK_RESULT(f, err_str)																	    \
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cerr << err_str << std::endl;                                                              \
		assert(res == VK_SUCCESS);																		\
	}																									\
}
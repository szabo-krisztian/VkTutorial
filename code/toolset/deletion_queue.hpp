#pragma once

#include <functional>
#include <deque>

namespace tlr
{
    
struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void PushFunction(std::function<void()>&& function)
    {
		deletors.push_back(function);
	}

	void Flush()
    {
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
		{
			(*it)();
		}

		deletors.clear();
	}
};

} // namespace tlr
#pragma once

#include "my_instance.hpp"
#include "my_window.hpp"

#include <optional>
#include <set>

namespace tlr
{

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

class MyVulkanDevice
{
public:
    MyVulkanDevice(MyVulkanInstance& instance, MyVulkanWindow& window);
    ~MyVulkanDevice();


private:
    const std::vector<const char*> M_VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    MyVulkanInstance& mInstance;
    MyVulkanWindow& mWindow;
    VkPhysicalDevice mPhysicalDevice;
    QueueFamilyIndices mFamilyIndices;
    VkDevice mLogicalDevice;
    VkQueue mGraphicsQueue;
    VkQueue mPresentQueue;

#ifdef NDEBUG
    const bool M_ENABLE_VALIDATION_LAYERS = false;
#else
    const bool M_ENABLE_VALIDATION_LAYERS = true;
#endif

    VkPhysicalDevice GetPhysicalDevice();
    bool IsPhysicalDeviceSuitable(const VkPhysicalDevice& device);
    QueueFamilyIndices GetQueueFamilyIndices(const VkPhysicalDevice& device);
    std::vector<VkQueueFamilyProperties> GetQueueFamilies(const VkPhysicalDevice& physicalDevice);
    VkResult InitLogicalDevice();
};

} // namespace tlr
#include "my_instance.hpp"

#include <iostream>
#include <vector>


int main()
{
    tlr::MyVulkanInstance instance;

    uint32_t physicalDevicesCount = 0;
    vkEnumeratePhysicalDevices(instance.Get(), &physicalDevicesCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(instance.Get(), &physicalDevicesCount, physicalDevices.data());

    for (const auto& device : physicalDevices)
    {
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        std::cout << deviceProperties.deviceName << std::endl;
    }

    return 0;
}
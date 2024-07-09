add_library(AppBase app_base.cpp)

add_subdirectory(instance-builder)
add_subdirectory(physical-device-selector)
add_subdirectory(device-builder)
add_subdirectory(swapchain-builder)

target_include_directories(AppBase PUBLIC instance-builder)
target_include_directories(AppBase PUBLIC swapchain-builder)

# when we link swapchain to AppBase, it will include the directories physical-device-selector, device-builder
target_link_libraries(AppBase PUBLIC
    InstanceBuilder
    PhysicalDeviceSelector
    DeviceBuilder
    SwapchainBuilder
)
add_library(AppBase app_base.cpp)

add_subdirectory(instance-builder)
target_include_directories(AppBase PUBLIC instance-builder)

add_subdirectory(physical-device-selector)
target_include_directories(AppBase PUBLIC physical-device-selector)

add_subdirectory(device-builder)
target_include_directories(AppBase PUBLIC device-builder)

add_subdirectory(swapchain-builder)
target_include_directories(AppBase PUBLIC swapchain-builder)

target_link_libraries(AppBase PRIVATE
    InstanceBuilder
    PhysicalDeviceSelector
    DeviceBuilder
    SwapchainBuilder
)
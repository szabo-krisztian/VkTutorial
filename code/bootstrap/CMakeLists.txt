add_library(AppBase app_base.cpp)

add_subdirectory(instance-builder)
add_subdirectory(physical-device-selector)
add_subdirectory(device-builder)
add_subdirectory(swapchain-builder)

target_include_directories(AppBase
    PUBLIC instance-builder
           physical-device-selector
           device-builder
           swapchain-builder
)

target_link_libraries(AppBase
    PRIVATE InstanceBuilder
            PhysicalDeviceSelector
            DeviceBuilder
            SwapchainBuilder
            Toolset
    PUBLIC GLFW_VULKAN_GLM
)
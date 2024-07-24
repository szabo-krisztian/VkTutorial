add_subdirectory(instance-builder)
add_subdirectory(physical-device-selector)
add_subdirectory(device-builder)
add_subdirectory(swapchain-builder)

add_library(AppBase app_base.cpp)
target_include_directories(AppBase
    PUBLIC instance-builder
           physical-device-selector
           device-builder
           swapchain-builder
)
target_link_libraries(AppBase
    PUBLIC GLFW_VULKAN_GLM  
           InputManager
    PRIVATE InstanceBuilder
            PhysicalDeviceSelector
            DeviceBuilder
            SwapchainBuilder
            Toolset
)
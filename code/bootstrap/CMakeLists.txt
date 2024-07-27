add_subdirectory(instance-builder)
add_subdirectory(physical-device-selector)
add_subdirectory(device-builder)
add_subdirectory(swapchain-builder)
add_subdirectory(input-manager)
add_subdirectory(camera)
add_subdirectory(timer)

add_library(AppBase app_base.cpp)
target_include_directories(AppBase
    PUBLIC instance-builder
           physical-device-selector
           device-builder
           swapchain-builder
           input-manager
           camera
           timer
)
target_link_libraries(AppBase
    PUBLIC GLFW_VULKAN_GLM  
           InputManager
           Camera
           Timer
    PRIVATE InstanceBuilder
            PhysicalDeviceSelector
            DeviceBuilder
            SwapchainBuilder
            Toolset
)
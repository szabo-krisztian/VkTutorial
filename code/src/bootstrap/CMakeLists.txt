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
           ${Vulkan_INCLUDE_DIRS}
           ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(AppBase
    PRIVATE InstanceBuilder
            PhysicalDeviceSelector
            DeviceBuilder
            SwapchainBuilder
    PUBLIC ${Vulkan_LIBRARIES}
           ${GLFW_LIBRARIESS}
)
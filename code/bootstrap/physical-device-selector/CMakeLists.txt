add_library(PhysicalDeviceSelector physical_device_selector.cpp)
target_link_libraries(PhysicalDeviceSelector 
    PUBLIC GLFW_VULKAN_GLM
)
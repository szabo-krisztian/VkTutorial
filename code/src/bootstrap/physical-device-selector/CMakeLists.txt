add_library(PhysicalDeviceSelector physical_device_selector.cpp)

target_include_directories(PhysicalDeviceSelector
    PUBLIC ${Vulkan_INCLUDE_DIRS}
           ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(PhysicalDeviceSelector
    PUBLIC ${Vulkan_LIBRARIES}
           ${GLFW_LIBRARIES}
)
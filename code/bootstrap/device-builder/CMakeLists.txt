add_library(Device device.cpp)
target_include_directories(Device
    PUBLIC ${BOOTSTRAP_DIR}/physical-device-selector
)
target_link_libraries(Device
    PUBLIC GLFW_VULKAN_GLM
           Buffer
    PRIVATE Toolset
)

add_library(DeviceBuilder device_builder.cpp)
target_include_directories(DeviceBuilder
    PUBLIC ${BOOTSTRAP_DIR}/physical-device-selector
)
target_link_libraries(DeviceBuilder
    PUBLIC GLFW_VULKAN_GLM    
           Device
    PRIVATE Toolset
)
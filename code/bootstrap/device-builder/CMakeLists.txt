add_library(DeviceBuilder device_builder.cpp)
target_include_directories(DeviceBuilder
    PUBLIC ${BOOTSTRAP_DIR}/physical-device-selector
)
target_link_libraries(DeviceBuilder
    PUBLIC GLFW_VULKAN_GLM    
    PRIVATE Toolset
)
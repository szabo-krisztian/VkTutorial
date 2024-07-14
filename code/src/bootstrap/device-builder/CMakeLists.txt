add_library(DeviceBuilder device_builder.cpp)

target_include_directories(DeviceBuilder
    PUBLIC ${CMAKE_SOURCE_DIR}/src/bootstrap/physical-device-selector
)

target_link_libraries(DeviceBuilder
    PRIVATE Toolset
    PUBLIC GLFW_VULKAN_GLM
)
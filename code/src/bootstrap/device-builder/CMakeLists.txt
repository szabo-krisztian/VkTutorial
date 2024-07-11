add_library(DeviceBuilder device_builder.cpp)

target_include_directories(DeviceBuilder
    PUBLIC ${Vulkan_INCLUDE_DIRS}
           ${GLFW_INCLUDE_DIRS}
           ${CMAKE_SOURCE_DIR}/src/bootstrap/physical-device-selector
)

target_link_libraries(DeviceBuilder
    PUBLIC ${Vulkan_LIBRARIES}
           ${GLFW_LIBRARIES}
    PRIVATE Toolset
)
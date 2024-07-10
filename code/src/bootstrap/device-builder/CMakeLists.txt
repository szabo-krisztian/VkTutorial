add_library(DeviceBuilder device_builder.cpp)

target_include_directories(DeviceBuilder
    PUBLIC ${Vulkan_INCLUDE_DIRS}
           ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(DeviceBuilder
    PUBLIC ${Vulkan_LIBRARIES}
           ${GLFW_LIBRARIES}
)

# device_builder.hpp needs to see physical_device.hpp
target_include_directories(DeviceBuilder
    PRIVATE ${CMAKE_SOURCE_DIR}/src/bootstrap/physical-device-selector
)
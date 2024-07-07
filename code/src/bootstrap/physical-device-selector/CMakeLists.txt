add_library(PhysicalDeviceSelector physical_device_selector.cpp)

target_include_directories(PhysicalDeviceSelector PUBLIC
    ${CMAKE_SOURCE_DIR}/src/bootstrap
)
target_include_directories(AppBase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(PhysicalDeviceSelector physical_device_selector.cpp)

target_include_directories(PhysicalDeviceSelector PRIVATE
    ${CMAKE_SOURCE_DIR}/src/bootstrap
)
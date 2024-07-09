target_include_directories(AppBase PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(DeviceBuilder device_builder.cpp)

target_include_directories(DeviceBuilder PRIVATE
    ${CMAKE_SOURCE_DIR}/src/bootstrap
    ${CMAKE_SOURCE_DIR}/src/bootstrap/physical-device-selector
)
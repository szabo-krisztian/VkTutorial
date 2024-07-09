add_library(DeviceBuilder device_builder.cpp)

target_include_directories(DeviceBuilder
    PUBLIC ${CMAKE_SOURCE_DIR}/src/bootstrap/physical-device-selector
)
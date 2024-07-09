add_library(SwapchainBuilder swapchain_builder.cpp)

target_include_directories(SwapchainBuilder
    PUBLIC ${CMAKE_SOURCE_DIR}/src/bootstrap/device-builder
           ${CMAKE_SOURCE_DIR}/src/bootstrap/physical-device-selector
)
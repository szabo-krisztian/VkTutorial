target_include_directories(AppBase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(SwapchainBuilder swapchain_builder.cpp)

target_include_directories(SwapchainBuilder PRIVATE
    ${CMAKE_SOURCE_DIR}/src/bootstrap
    ${CMAKE_SOURCE_DIR}/src/bootstrap/device-builder
    ${CMAKE_SOURCE_DIR}/src/bootstrap/physical-device-selector
)
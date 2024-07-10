add_library(SwapchainBuilder swapchain_builder.cpp)

target_include_directories(SwapchainBuilder
    PUBLIC ${Vulkan_INCLUDE_DIRS}
           ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(SwapchainBuilder
    PUBLIC ${Vulkan_LIBRARIES}
           ${GLFW_LIBRARIES}
)

# swapchain_builder.hpp needs to see physical_device.hpp, device.hpp
target_include_directories(SwapchainBuilder
    PUBLIC ${CMAKE_SOURCE_DIR}/src/bootstrap/physical-device-selector
           ${CMAKE_SOURCE_DIR}/src/bootstrap/device-builder           
)
add_library(SwapchainBuilder swapchain_builder.cpp)
target_include_directories(SwapchainBuilder
    PUBLIC ${BOOTSTRAP_DIR}/physical-device-selector
	       ${BOOTSTRAP_DIR}/device-builder
)
target_link_libraries(SwapchainBuilder
    PUBLIC GLFW_VULKAN_GLM
    PRIVATE Toolset
)
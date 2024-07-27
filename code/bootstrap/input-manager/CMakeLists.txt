add_library(InputManager input_manager.cpp)
target_link_libraries(InputManager
    PUBLIC GLFW_VULKAN_GLM
)
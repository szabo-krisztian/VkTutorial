add_library(Camera camera.cpp)
target_link_libraries(Camera
    PUBLIC GLFW_VULKAN_GLM
)
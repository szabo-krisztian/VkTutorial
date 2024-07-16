add_library(Toolset toolset.cpp)
target_link_libraries(Toolset
    PUBLIC GLFW_VULKAN_GLM
)

add_library(ShaderModule shader_module.cpp)
target_link_libraries(ShaderModule
    PUBLIC GLFW_VULKAN_GLM
    PRIVATE Toolset
)
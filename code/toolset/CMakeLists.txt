add_library(Toolset toolset.cpp)
target_include_directories(Toolset
    PUBLIC ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Toolset
    PUBLIC ${Vulkan_LIBRARIES}
)

add_library(ShaderModule shader_module.cpp)
target_link_libraries(ShaderModule
    PUBLIC GLFW_VULKAN_GLM
    PRIVATE Toolset
)
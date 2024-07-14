add_library(InstanceBuilder instance_builder.cpp)

target_link_libraries(InstanceBuilder
    PRIVATE Toolset
    PUBLIC GLFW_VULKAN_GLM
)
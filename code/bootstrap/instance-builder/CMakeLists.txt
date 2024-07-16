add_library(InstanceBuilder instance_builder.cpp)
target_link_libraries(InstanceBuilder
    PUBLIC GLFW_VULKAN_GLM    
    PRIVATE Toolset
)
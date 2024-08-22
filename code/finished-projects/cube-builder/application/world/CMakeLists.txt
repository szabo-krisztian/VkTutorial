add_library(Block block.cpp)
target_link_libraries(Block
    PUBLIC GLFW_VULKAN_GLM
)

add_library(World world.cpp)
target_link_libraries(World
    PUBLIC GLFW_VULKAN_GLM
           Block
)
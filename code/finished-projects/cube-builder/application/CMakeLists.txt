add_library(World world.cpp)
target_link_directories(World
    PUBLIC GLFW_VULKAN_GLM
)

add_library(App app.cpp)
target_include_directories(App
    PUBLIC ${BOOTSTRAP_DIR}
           shaders
)

target_link_libraries(App
    PUBLIC GLFW_VULKAN_GLM
           AppBase
    PRIVATE Toolset
            ShaderModule
            Buffer
            World
)
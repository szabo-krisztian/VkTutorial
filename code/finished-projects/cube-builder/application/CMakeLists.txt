add_subdirectory(world)

add_library(App app.cpp)
target_include_directories(App
    PUBLIC ${BOOTSTRAP_DIR}
           shaders
           world
)

target_link_libraries(App
    PUBLIC GLFW_VULKAN_GLM
           AppBase
    PRIVATE Toolset
            ShaderModule
            Buffer
            World
)
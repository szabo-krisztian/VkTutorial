add_subdirectory(simulator)

add_library(App app.cpp)
target_include_directories(App
    PUBLIC ${BOOTSTRAP_DIR}
           shaders
           simulator
)

target_link_libraries(App
    PUBLIC GLFW_VULKAN_GLM
           AppBase
           Simulator
    PRIVATE Toolset
            ShaderModule
            Buffer
)
add_library(App app.cpp)
target_include_directories(App
    PUBLIC ${BOOTSTRAP_DIR}
           ${TOOLSET_DIR}
           shaders
)

target_link_libraries(App
    PUBLIC GLFW_VULKAN_GLM
           AppBase
    PUBLIC  Toolset
            ShaderModule
            Buffer
)
add_library(ShaderModule shader_module.cpp)

target_link_libraries(ShaderModule
    PUBLIC GLFW_VULKAN_GLM
    PRIVATE Toolset
)

add_library(App app.cpp)

target_include_directories(App
    PUBLIC ${CMAKE_SOURCE_DIR}/src/bootstrap
)

target_link_libraries(App
    PUBLIC AppBase
    PRIVATE Toolset ShaderModule
)
add_library(Camera camera.cpp)

add_library(App app.cpp)
target_include_directories(App
    PUBLIC ${BOOTSTRAP_DIR}
)

target_link_libraries(App
    PUBLIC GLFW_VULKAN_GLM
           AppBase
           Camera
    PRIVATE Toolset
)
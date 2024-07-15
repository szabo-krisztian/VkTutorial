add_library(App app.cpp)
target_include_directories(App
    PUBLIC ${BOOTSTRAP_DIR}
           shaders
)

target_link_libraries(App
    PUBLIC AppBase
    PRIVATE Toolset ShaderModule
)
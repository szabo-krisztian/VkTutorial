add_library(ShaderModule shader_module.cpp)
target_include_directories(ShaderModule
    PUBLIC ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(ShaderModule
    PUBLIC ${Vulkan_LIBRARIES}
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
add_library(InstanceBuilder instance_builder.cpp)

target_include_directories(InstanceBuilder
    PUBLIC ${Vulkan_INCLUDE_DIRS}
           ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(InstanceBuilder
    PUBLIC ${Vulkan_LIBRARIES}
           ${GLFW_LIBRARIES}
    PRIVATE Toolset
)
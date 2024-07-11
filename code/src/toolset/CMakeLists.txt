add_library(Initializers initializers.cpp)

target_include_directories(Initializers
    PUBLIC ${Vulkan_INCLUDE_DIRS}
           ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(Initializers
    PUBLIC ${Vulkan_LIBRARIES}
           ${GLFW_LIBRARIES}
)
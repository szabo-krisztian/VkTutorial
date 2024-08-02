if (NOT DEFINED GLM_PATH)
    message(FATAL_ERROR "GLM path not defined!")
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(${GLM_PATH} ${CMAKE_BINARY_DIR}/glm)

add_library(GLFW_VULKAN_GLM INTERFACE)
target_include_directories(GLFW_VULKAN_GLM
    INTERFACE
        $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
        ${Vulkan_INCLUDE_DIRS}
        $<TARGET_PROPERTY:glm-header-only,INTERFACE_INCLUDE_DIRECTORIES>
)
target_link_libraries(GLFW_VULKAN_GLM
    INTERFACE
        glfw
        ${Vulkan_LIBRARIES}
        glm-header-only
)

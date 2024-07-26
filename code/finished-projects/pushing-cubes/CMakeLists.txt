# My build command:
# cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake/glfw3" -DGLM_PATH="C:/glm"

cmake_minimum_required(VERSION 3.10)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif()

find_package(glfw3 REQUIRED)
if (NOT glfw3_FOUND)
    message(FATAL_ERROR "GLFW3 not found!")
endif()
get_target_property(GLFW_INCLUDE_DIRS glfw INTERFACE_INCLUDE_DIRECTORIES)

if (DEFINED GLM_PATH)
    add_subdirectory(${GLM_PATH} ${CMAKE_BINARY_DIR}/glm)
else()
    message(FATAL_ERROR "GLM_PATH not defined!")
endif()

add_library(GLFW_VULKAN_GLM INTERFACE)
target_include_directories(GLFW_VULKAN_GLM
    INTERFACE ${GLFW_INCLUDE_DIRS}
              ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(GLFW_VULKAN_GLM
    INTERFACE glfw
              ${Vulkan_LIBRARIES}
              glm-header-only
)

set(TOOLSET_DIR ${CMAKE_SOURCE_DIR}/../../toolset)
set(BOOTSTRAP_DIR ${CMAKE_SOURCE_DIR}/../../bootstrap)

include_directories(${TOOLSET_DIR})
add_subdirectory(${TOOLSET_DIR} ${CMAKE_BINARY_DIR}/toolset)
add_subdirectory(${BOOTSTRAP_DIR} ${CMAKE_BINARY_DIR}/bootstrap)
add_subdirectory(application)

add_executable(Main main.cpp)

target_include_directories(Main
    PRIVATE application
)
target_link_libraries(Main
    PRIVATE App
)
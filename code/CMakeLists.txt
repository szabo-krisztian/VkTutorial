# If the find_package functions do not work, try including paths explicit with
# cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake/glfw3"

cmake_minimum_required(VERSION 3.10)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Search for Vulkan dependencies
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif()

# Search for GLFW3 dependencies
find_package(GLFW3 REQUIRED)
if (NOT GLFW3_FOUND)
    message(FATAL_ERROR "GLFW3 not found!")
endif()
get_target_property(GLFW_INCLUDE_DIRS glfw INTERFACE_INCLUDE_DIRECTORIES)
set(GLFW_LIBRARIES glfw)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MD")
endif()

# Search for GLM dependencies (it is header-only)
set(GLM_PATH "C:/glm")
add_subdirectory(${GLM_PATH} ${CMAKE_BINARY_DIR}/glm)

# Each library in the application that uses any of these extenernal dependencies are
# going to be linked against GLFW_VULKAN_GLM
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

# Every module sees these directories for convenience.
include_directories(${CMAKE_SOURCE_DIR}/src/toolset)

add_executable(Main src/main.cpp)

add_subdirectory(src)
target_include_directories(Main
    PRIVATE src/application
)
target_link_libraries(Main
    PRIVATE App
)
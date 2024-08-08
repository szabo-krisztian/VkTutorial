# My build command:
# cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake/glfw3" -DGLM_PATH="C:/glm" -DTINYLOADER_PATH="C:/tinyobjloader"

cmake_minimum_required(VERSION 3.10)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
endif()

set(MY_CMAKE_DIR ../../cmake)
include(${MY_CMAKE_DIR}/glfw_vulkan_glm.cmake)
include(${MY_CMAKE_DIR}/essentials.cmake)

add_subdirectory(application)
add_executable(Main main.cpp)

target_include_directories(Main PRIVATE application)
target_link_libraries(Main PRIVATE App)
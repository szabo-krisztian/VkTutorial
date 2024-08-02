# My build command:
# cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake/glfw3" -DGLM_PATH="C:/glm"

cmake_minimum_required(VERSION 3.10)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MD")
endif()

set(MY_CMAKE_DIR ../../cmake)
include(${MY_CMAKE_DIR}/GLFW_Vulkan_GLM.cmake)
include(${MY_CMAKE_DIR}/Essentials.cmake)

add_subdirectory(application)
add_executable(Main main.cpp)

target_include_directories(Main PRIVATE application)
target_link_libraries(Main PRIVATE App)
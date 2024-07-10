# cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake/glfw3"

cmake_minimum_required(VERSION 3.10)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif()

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

# every module sees these directories for convenience. in the future only subdirectories will see these
include_directories(${CMAKE_SOURCE_DIR}/src/bootstrap)

add_executable(Main src/main.cpp)
add_subdirectory(src)

target_include_directories(Main
    PRIVATE src/application
)

target_link_libraries(Main
    PRIVATE App
)
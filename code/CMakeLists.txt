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
get_target_property(GLFW_INCLUDE_DIR glfw INTERFACE_INCLUDE_DIRECTORIES)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MD")
endif()

include_directories(${Vulkan_INCLUDE_DIRS} ${GLFW_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/src/bootstrap)
add_executable(Main src/main.cpp)
add_subdirectory(src)

target_include_directories(Main PRIVATE src/bootstrap/physical-device-selector)

target_link_libraries(Main PRIVATE
    ${Vulkan_LIBRARIES}
    glfw
    AppBase
)
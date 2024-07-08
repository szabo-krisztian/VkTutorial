cmake_minimum_required(VERSION 3.10)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_INCLUDE_DIR "C:/CppLibraries/glfw-3.4/Include")
set(GLFW_LIB_DIR "C:/CppLibraries/glfw-3.4/build/src/Release")

set(VULKAN_INCLUDE_DIR "C:/VulkanAPI/Include")
set(VULKAN_LIB_DIR "C:/VulkanAPI/Lib")

find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif()

find_library(GLFW_LIBRARY glfw3 PATHS ${GLFW_LIB_DIR})
if(NOT GLFW_LIBRARY)
    message(FATAL_ERROR "GLFW library not found!")
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MD")
endif()

include_directories(${VULKAN_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include)
add_executable(MyVulkanProject src/main.cpp)
add_subdirectory(src)

target_link_libraries(MyVulkanProject PRIVATE
    ${Vulkan_LIBRARIES}
    ${GLFW_LIBRARY}
    AppBase
)
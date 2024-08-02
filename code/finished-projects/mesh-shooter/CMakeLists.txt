# My build command:
# cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake/glfw3" -DGLM_PATH="C:/glm" -DPHYSX_INCLUDE="C:/PhysX-106.0-physx-5.4.1/physx/include" -DPHYSX_LIB="C:/PhysX-106.0-physx-5.4.1/physx/bin/win.x86_64.vc142.mt/checked"
# cmake --build . --config Release

cmake_minimum_required(VERSION 3.10)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(
        "$<$<CONFIG:Debug>:/MTd>"
        "$<$<CONFIG:Release>:/MT>"
    )
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(MY_CMAKE_DIR ../../cmake)
include(${MY_CMAKE_DIR}/glfw_vulkan_glm.cmake)
include(${MY_CMAKE_DIR}/essentials.cmake)
include(${MY_CMAKE_DIR}/physx.cmake)

add_subdirectory(application)
add_executable(Main main.cpp)

target_include_directories(Main PRIVATE application)
target_link_libraries(Main PRIVATE App msvcrt)
# My build command:
# cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake/glfw3" 
#          -DGLM_PATH="C:/glm" -DPHYSX_INCLUDE="C:/PhysX-106.0-physx-5.4.1/physx/include"
#          -DPHYSX_LIB="C:/PhysX-106.0-physx-5.4.1/physx/bin/win.x86_64.vc142.mt/checked"
# cmake --build . --config Release

cmake_minimum_required(VERSION 3.10)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT DEFINED GLM_PATH)
    message(FATAL_ERROR "GLM path not defined!")
endif()

if (NOT DEFINED PHYSX_INCLUDE)
    message(FATAL_ERROR "PhysX include path not defined!")
endif()

if (NOT DEFINED PHYSX_LIB)
    message(FATAL_ERROR "PhysX lib path not defined!")
endif()

if (MSVC)
    add_compile_options(
        "$<$<CONFIG:Debug>:/MTd>"
        "$<$<CONFIG:Release>:/MT>"
    )
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
add_subdirectory(${GLM_PATH} ${CMAKE_BINARY_DIR}/glm)

add_library(GLFW_VULKAN_GLM INTERFACE)
target_include_directories(GLFW_VULKAN_GLM
    INTERFACE
        $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
        ${Vulkan_INCLUDE_DIRS}
        glm-header-only
)
target_link_libraries(GLFW_VULKAN_GLM
    INTERFACE
        glfw
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

add_library(PHYSX INTERFACE)
target_include_directories(PHYSX INTERFACE ${PHYSX_INCLUDE})
target_link_directories(PHYSX INTERFACE ${PHYSX_LIB})
target_link_libraries(PHYSX
    INTERFACE
        PhysXExtensions_static_64
        PhysX_64
        PhysXPvdSDK_static_64
        PhysXCommon_64
        PhysXFoundation_64
        PhysXCooking_64
)

target_include_directories(Main PRIVATE application)
target_link_libraries(Main PRIVATE App msvcrt)
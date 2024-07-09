target_include_directories(AppBase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(InstanceBuilder instance_builder.cpp)

target_include_directories(InstanceBuilder PRIVATE
    ${CMAKE_SOURCE_DIR}/src/bootstrap
)
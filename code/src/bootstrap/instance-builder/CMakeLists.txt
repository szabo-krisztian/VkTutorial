add_library(InstanceBuilder instance_builder.cpp)

target_include_directories(InstanceBuilder PUBLIC
    ${CMAKE_SOURCE_DIR}/src/bootstrap
)
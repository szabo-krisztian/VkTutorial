add_library(App app.cpp)

target_include_directories(App
    PUBLIC ${CMAKE_SOURCE_DIR}/src/bootstrap
)

target_link_libraries(App
    PUBLIC AppBase
    PRIVATE Initializers
)
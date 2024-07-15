add_library(App app.cpp)
target_include_directories(App
    PUBLIC ${BOOTSTRAP_DIR}
)

target_link_libraries(App
    PUBLIC AppBase
    PRIVATE Toolset
)
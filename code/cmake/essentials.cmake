set(TOOLSET_DIR ${CMAKE_CURRENT_LIST_DIR}/../toolset)
set(BOOTSTRAP_DIR ${CMAKE_CURRENT_LIST_DIR}/../bootstrap)

include_directories(${TOOLSET_DIR})
add_subdirectory(${TOOLSET_DIR} ${CMAKE_BINARY_DIR}/toolset)
add_subdirectory(${BOOTSTRAP_DIR} ${CMAKE_BINARY_DIR}/bootstrap)
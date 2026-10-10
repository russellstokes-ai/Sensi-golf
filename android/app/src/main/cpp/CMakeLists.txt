cmake_minimum_required(VERSION 3.22.1)
project(sensigolf_android LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Same recovered engine linked on Android; NEVER a second physics model.
get_filename_component(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../" ABSOLUTE)
add_subdirectory("${REPO_ROOT}/engine" "${CMAKE_CURRENT_BINARY_DIR}/sensigolf-engine")

add_library(sensigolf_android SHARED native_bridge.cpp)
target_link_libraries(sensigolf_android PRIVATE sensigolf_core)
target_include_directories(sensigolf_android PRIVATE "${REPO_ROOT}/engine/include")

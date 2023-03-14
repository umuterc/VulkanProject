cmake_minimum_required(VERSION 3.21)

project(vulkan-practices)
set (CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_BINARY_DIR}/src)

add_subdirectory(externals)
add_subdirectory(src)
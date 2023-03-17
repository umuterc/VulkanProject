cmake_minimum_required(VERSION 3.21)
project(VulkanProject)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_BINARY_DIR}/src)

add_subdirectory(externals)
add_subdirectory(src)
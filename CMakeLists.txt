cmake_minimum_required(VERSION 3.21)
project(VulkanProject)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_BINARY_DIR}/src)

if (WIN32)
    add_custom_target(shader_compiler
        COMMAND cmd /c "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/compile_win32.bat"
        COMMENT "Compiling shaders for Windows"
    )
elseif(APPLE)
    add_custom_target(shader_compiler
        COMMAND sh "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/compile_mac.sh"
        COMMENT "Compiling shaders for Mac"
    )
endif()

add_subdirectory(externals)
add_subdirectory(src)
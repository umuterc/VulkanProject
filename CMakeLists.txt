cmake_minimum_required(VERSION 3.21)
project(VulkanProject)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_BINARY_DIR}/src)

if (WIN32)
    execute_process(COMMAND cmd /c "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/compile_win32.bat"
        OUTPUT_QUIET
        ERROR_QUIET
        RESULT_VARIABLE result
    )
    if (result)
        message(FATAL_ERROR "Failed to compile shaders")
    endif()
elseif(APPLE)
    execute_process(COMMAND sh -c "/Users/umutercan/VulkanSDK/1.3.239.0/macOS/bin/glslc ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader.vert -o ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/vert.spv && /Users/umutercan/VulkanSDK/1.3.239.0/macOS/bin/glslc ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader.frag -o ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/frag.spv"
        OUTPUT_QUIET
        ERROR_QUIET
        RESULT_VARIABLE result
    )
    if (result)
        message(FATAL_ERROR "Failed to compile shaders")
    endif()
endif()

add_subdirectory(externals)
add_subdirectory(src)

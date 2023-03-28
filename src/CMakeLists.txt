file(GLOB_RECURSE SOURCE_FILES 
    "./*"
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} 
    PUBLIC "./"
    ${CMAKE_SOURCE_DIR}/externals/glfw/include
    ${CMAKE_SOURCE_DIR}/externals/glm/include
    ${CMAKE_SOURCE_DIR}/assets
)
find_package(Vulkan REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME} Vulkan::Vulkan glfw glm)
add_executable (vulkan002 main.cpp)
add_custom_command(TARGET vulkan002 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(vulkan002 LINK_PUBLIC framework ${VULKAN_DYNAMIC_LIBRARY})

add_executable (vulkan003 main.cpp)
add_custom_command(TARGET vulkan003 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(vulkan003 LINK_PUBLIC framework vulkan-1)

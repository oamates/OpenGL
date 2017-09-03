add_executable (vulkan006 main.cpp)
add_custom_command(TARGET vulkan006 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(vulkan006 LINK_PUBLIC framework vulkan-1)

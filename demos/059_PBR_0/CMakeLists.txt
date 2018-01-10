add_executable (pbr0 pbr0.cpp)
add_custom_command(TARGET pbr0 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(pbr0 LINK_PUBLIC framework)
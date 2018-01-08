add_executable (pbr1 pbr.cpp)
add_custom_command(TARGET pbr1 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(pbr1 LINK_PUBLIC framework)
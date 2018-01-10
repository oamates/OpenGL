add_executable (pbr2 pbr2.cpp)
add_custom_command(TARGET pbr2 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(pbr2 LINK_PUBLIC framework)
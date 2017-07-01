add_executable (sdf_tex main.cpp)

add_custom_command(TARGET sdf_tex POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(sdf_tex LINK_PUBLIC framework)

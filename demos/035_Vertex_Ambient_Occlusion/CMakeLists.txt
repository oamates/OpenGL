add_executable (vao vao.cpp)
add_custom_command(TARGET vao POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(vao LINK_PUBLIC framework)

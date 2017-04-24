add_executable (gl046 main.cpp)
add_custom_command(TARGET gl046 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl046 LINK_PUBLIC framework)
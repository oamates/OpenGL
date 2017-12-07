add_executable (rt1 main.cpp)
add_custom_command(TARGET rt1 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(rt1 LINK_PUBLIC framework)

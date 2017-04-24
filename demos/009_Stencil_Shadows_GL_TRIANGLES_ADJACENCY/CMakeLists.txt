add_executable (gl009 main.cpp)

add_custom_command(TARGET gl009 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl009 LINK_PUBLIC framework)
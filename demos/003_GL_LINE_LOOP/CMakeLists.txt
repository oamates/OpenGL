add_executable (gl003 main.cpp)

add_custom_command(TARGET gl003 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl003 LINK_PUBLIC framework)
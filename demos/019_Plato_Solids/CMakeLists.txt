add_executable (gl019 main.cpp)
add_custom_command(TARGET gl019 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl019 LINK_PUBLIC framework)

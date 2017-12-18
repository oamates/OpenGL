add_executable (gl070 main.cpp solid.cpp)

add_custom_command(TARGET gl070 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl070 LINK_PUBLIC framework)

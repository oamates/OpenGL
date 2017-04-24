add_executable (gl075 main.cpp)

add_custom_command(TARGET gl075 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl075 LINK_PUBLIC framework)
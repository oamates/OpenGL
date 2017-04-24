add_executable (gl043 main.cpp)

add_custom_command(TARGET gl043 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl043 LINK_PUBLIC framework)

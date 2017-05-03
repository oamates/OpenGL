add_executable (gl081 conformal_map.cpp)

add_custom_command(TARGET gl081 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl081 LINK_PUBLIC framework)

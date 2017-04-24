add_executable (heightmap main.cpp)

add_custom_command(TARGET heightmap POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(heightmap LINK_PUBLIC framework)

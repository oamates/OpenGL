add_executable (ray_tracer4 main.cpp)

add_custom_command(TARGET ray_tracer4 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(ray_tracer4 LINK_PUBLIC framework)

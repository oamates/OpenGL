add_executable (ray_tracer main.cpp Camera.cpp)
add_custom_command(TARGET ray_tracer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(ray_tracer LINK_PUBLIC framework)

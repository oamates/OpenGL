add_executable (ray_tracer5 engine.cpp loader.cpp main.cpp shader.cpp)

add_custom_command(TARGET ray_tracer5 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)
add_custom_command(TARGET ray_tracer5 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/make ${CMAKE_CURRENT_BINARY_DIR}/mesh)

target_link_libraries(ray_tracer5 LINK_PUBLIC framework)

add_executable (transform_fb main.cpp)

add_custom_command(TARGET transform_fb POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(transform_fb LINK_PUBLIC framework)

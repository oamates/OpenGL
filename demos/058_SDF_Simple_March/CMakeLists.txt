add_executable (simple_march main.cpp)

add_custom_command(TARGET simple_march POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(simple_march LINK_PUBLIC framework -latomic)

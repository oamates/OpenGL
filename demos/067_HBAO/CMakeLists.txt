add_executable (gl067 main.cpp geometry.cpp fbo2d.cpp mtl.cpp obj1.cpp surface1.cpp)

add_custom_command(TARGET gl067 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl067 LINK_PUBLIC framework)

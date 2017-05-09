add_executable (gl050 raymarch.cpp)

add_custom_command(TARGET gl050 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl050 LINK_PUBLIC framework imgui)

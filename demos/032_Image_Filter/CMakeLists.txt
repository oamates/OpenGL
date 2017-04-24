add_executable (gl032 main.cpp)
add_custom_command(TARGET gl032 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl032 LINK_PUBLIC framework imgui)

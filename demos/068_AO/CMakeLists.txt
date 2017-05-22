add_executable (gl068 ao.cpp)
add_custom_command(TARGET gl068 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl068 LINK_PUBLIC framework)

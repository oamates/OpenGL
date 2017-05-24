add_executable (gl069 ao.cpp)
add_custom_command(TARGET gl069 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl069 LINK_PUBLIC framework imgui)

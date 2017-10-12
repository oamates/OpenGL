add_executable (gl056_aa raymarch.cpp)

add_custom_command(TARGET gl056_aa POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl056_aa LINK_PUBLIC framework imgui)

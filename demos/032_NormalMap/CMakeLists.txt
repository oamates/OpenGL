add_executable (normalmap main.cpp normalmap.cpp)
add_custom_command(TARGET normalmap POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(normalmap LINK_PUBLIC framework imgui)

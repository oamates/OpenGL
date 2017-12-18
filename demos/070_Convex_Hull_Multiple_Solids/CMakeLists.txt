add_executable (gl070_1 main.cpp solid.cpp)

add_custom_command(TARGET gl070_1 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl070_1 LINK_PUBLIC framework)

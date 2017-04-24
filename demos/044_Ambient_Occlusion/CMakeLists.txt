add_executable (gl044 main.cpp model.cpp shadowmap.cpp)
add_custom_command(TARGET gl044 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl044 LINK_PUBLIC framework)

add_executable (gl053 main.cpp fft.cpp)

add_custom_command(TARGET gl053 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl053 LINK_PUBLIC framework)

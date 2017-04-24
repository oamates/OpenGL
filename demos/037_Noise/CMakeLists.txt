add_executable (gl037 noise.cpp value.cpp simplex.cpp gradient.cpp main.cpp avx2.cpp sse2.cpp sse41.cpp)
add_custom_command(TARGET gl037 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl037 LINK_PUBLIC framework)
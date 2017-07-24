add_executable (sdf2 main.cpp makelevelset3.cpp)

add_custom_command(TARGET sdf2 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(sdf2 LINK_PUBLIC framework)

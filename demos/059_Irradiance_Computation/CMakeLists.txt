add_executable (irr irr.cpp)
add_custom_command(TARGET irr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(irr LINK_PUBLIC framework)
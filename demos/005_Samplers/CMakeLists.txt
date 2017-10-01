add_executable (sampler main.cpp shader_mgmt.cpp)

add_custom_command(TARGET sampler POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)
add_custom_command(TARGET sampler POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res)

target_link_libraries(sampler LINK_PUBLIC framework)
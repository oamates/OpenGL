add_executable (ao_cs_ws ao.cpp)
add_custom_command(TARGET ao_cs_ws POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(ao_cs_ws LINK_PUBLIC framework imgui)

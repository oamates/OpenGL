add_executable (gl082 main.cpp isosurface.cpp)

add_custom_command(TARGET gl082 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl082 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES})

add_executable (oculus004 main.cpp)

add_custom_command(TARGET oculus004 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(oculus004 LINK_PUBLIC ovr framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES})

add_executable (oculus005 main.cpp)

add_custom_command(TARGET oculus005 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)
add_custom_command(TARGET oculus005 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res)

target_link_libraries(oculus005 LINK_PUBLIC ovr framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES})

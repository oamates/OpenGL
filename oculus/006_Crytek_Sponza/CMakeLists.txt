add_executable (oculus006 main.cpp)

add_custom_command(TARGET oculus006 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)
add_custom_command(TARGET oculus006 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/crytek-sponza ${CMAKE_CURRENT_BINARY_DIR}/crytek-sponza)

target_link_libraries(oculus006 LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} OVR)

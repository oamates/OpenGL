add_executable (cms main.cpp octree.cpp)

add_custom_command(TARGET cms POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(cms LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES})

add_executable (cms address.cpp algcms.cpp array3d.cpp cell.cpp edge.cpp index3d.cpp isosurface.cpp main.cpp mesh.cpp octree.cpp point.cpp strip.cpp util.cpp vec3.cpp vertex.cpp)

add_custom_command(TARGET cms POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(cms LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES})

add_executable (ray_tracer6 camera.cpp helper.cpp main.cpp matrix3.cpp matrix4.cpp octree.cpp parser.cpp primitive.cpp renderer.cpp resourcemanager.cpp scene.cpp shadermanager.cpp stringutils.cpp vector3.cpp vector4.cpp)

#add_executable (load_scene loadscene.cpp)

add_custom_command(TARGET ray_tracer6 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)

target_link_libraries(ray_tracer6 LINK_PUBLIC framework)

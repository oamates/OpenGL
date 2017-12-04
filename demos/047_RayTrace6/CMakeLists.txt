add_executable (ray_tracer6 camera.cpp helper.cpp main.cpp octree.cpp parser.cpp primitive.cpp renderer.cpp resourcemanager.cpp scene.cpp shadermanager.cpp stringutils.cpp)

#add_executable (load_scene loadscene.cpp)

add_custom_command(TARGET ray_tracer6 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl   ${CMAKE_CURRENT_BINARY_DIR}/glsl)
add_custom_command(TARGET ray_tracer6 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/model  ${CMAKE_CURRENT_BINARY_DIR}/model)
add_custom_command(TARGET ray_tracer6 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/scenes ${CMAKE_CURRENT_BINARY_DIR}/scenes)

target_link_libraries(ray_tracer6 LINK_PUBLIC framework)

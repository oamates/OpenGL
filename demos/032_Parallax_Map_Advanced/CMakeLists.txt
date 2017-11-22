add_executable (parallax_mapa camera.cpp gl_aux.cpp light.cpp main.cpp mesh.cpp parallax.cpp tiny_obj_loader.cpp trackball_camera.cpp utilities.cpp)

add_custom_command(TARGET parallax_mapa POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res)
add_custom_command(TARGET parallax_mapa POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(parallax_mapa LINK_PUBLIC framework imgui)

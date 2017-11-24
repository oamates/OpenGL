add_executable (ray_tracer3 shaders.cpp timer.cpp obj.cpp main.cpp camera.cpp SOIL/image_DXT.c SOIL/image_helper.c SOIL/SOIL.c SOIL/stb_image_aug.c)

add_custom_command(TARGET ray_tracer3 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)
add_custom_command(TARGET ray_tracer3 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/sword ${CMAKE_CURRENT_BINARY_DIR}/sword)
add_custom_command(TARGET ray_tracer3 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bunny ${CMAKE_CURRENT_BINARY_DIR}/bunny)
add_custom_command(TARGET ray_tracer3 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/bth_logo_obj_tga ${CMAKE_CURRENT_BINARY_DIR}/bth_logo_obj_tga)

target_link_libraries(ray_tracer3 LINK_PUBLIC framework)

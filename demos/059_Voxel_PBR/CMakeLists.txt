add_executable (voxel_pbr main.cpp
                          behaviors/free_camera.cpp 
                          core/assets_manager.cpp core/engine.cpp
                          interfaces/main_ui.cpp
                          primitives/fullscreen_quad.cpp primitives/shapes.cpp
                          renderers/gi_deferred_renderer.cpp renderers/shadow_map_renderer.cpp renderers/voxelizer_renderer.cpp 
                          rendering/ui_renderer.cpp rendering/render_window.cpp 
                          scene/scene.cpp
                          types/transform.cpp
                          util/raw_format.cpp util/scene_importer.cpp)

add_custom_command(TARGET voxel_pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_custom_command(TARGET voxel_pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/fonts ${CMAKE_CURRENT_BINARY_DIR}/fonts)
add_custom_command(TARGET voxel_pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)
add_custom_command(TARGET voxel_pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/models ${CMAKE_CURRENT_BINARY_DIR}/models)

target_link_libraries(voxel_pbr LINK_PUBLIC framework imgui assimp)
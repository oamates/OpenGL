add_executable (pbr Animator.cpp Axis.cpp Camera.cpp Drawable.cpp Earth.cpp Light.cpp LinesGeometry.cpp LinesMaterial.cpp main.cpp Material.cpp 
                      Mesh.cpp Moon.cpp PhongMaterial.cpp Scene.cpp Shader.cpp Sphere.cpp SphereAnimator.cpp SphereMesh.cpp Timer.cpp)

add_custom_command(TARGET pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)
add_custom_command(TARGET pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_custom_command(TARGET pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/textures ${CMAKE_CURRENT_BINARY_DIR}/textures)

target_link_libraries(pbr LINK_PUBLIC framework)
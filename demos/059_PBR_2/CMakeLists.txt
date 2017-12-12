add_executable (pbr main.cpp animator.cpp axis.cpp camera.cpp drawable.cpp earth.cpp light.cpp linesgeometry.cpp linesmaterial.cpp material.cpp 
                    mesh.cpp moon.cpp phongmaterial.cpp scene.cpp shader.cpp sphere.cpp sphereanimator.cpp spheremesh.cpp timer.cpp)

add_custom_command(TARGET pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)
add_custom_command(TARGET pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders)
add_custom_command(TARGET pbr POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/textures ${CMAKE_CURRENT_BINARY_DIR}/textures)

target_link_libraries(pbr LINK_PUBLIC framework)
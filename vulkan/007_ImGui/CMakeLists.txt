add_executable (vulkan007 main.cpp imgui_impl_glfw_vulkan.cpp)
add_custom_command(TARGET vulkan007 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(vulkan007 LINK_PUBLIC framework vulkan-1 imgui)

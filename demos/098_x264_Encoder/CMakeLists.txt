add_executable (gl098 x264.cpp yuv420p.cpp main.cpp)

add_custom_command(TARGET gl098 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl098 LINK_PUBLIC framework ${FFMPEG_LIBRARIES} ${FFMPEG_DEPENDENCIES}) 




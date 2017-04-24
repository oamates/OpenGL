add_executable (gl099 player.cpp)

add_custom_command(TARGET gl099 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(gl099 LINK_PUBLIC framework ${FFMPEG_LIBRARIES} ${FFMPEG_DEPENDENCIES}) 




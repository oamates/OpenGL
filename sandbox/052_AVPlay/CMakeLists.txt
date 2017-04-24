#find and add all SDL package
pkg_search_module(SDL REQUIRED sdl)
include_directories(${SDL_INCLUDE_DIRS})

add_executable (avplayer avplayer.c cmdutils.c)
add_custom_command(TARGET avplayer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res)
add_custom_command(TARGET avplayer POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)
target_link_libraries(avplayer LINK_PUBLIC framework ${SDL_STATIC_LIBRARIES} ${LIBAV_STATIC_LIBRARIES})



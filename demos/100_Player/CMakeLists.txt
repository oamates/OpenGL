set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-deprecated-declarations -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-deprecated-declarations -g")

add_executable (player_1 player_1.cpp)
add_executable (player_2 player_2.cpp)
add_executable (player_3 player_3.cpp)
add_executable (player_4 player_4.cpp)
add_executable (player_5 player_5.cpp)

add_custom_command(TARGET player_1 POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/glsl ${CMAKE_CURRENT_BINARY_DIR}/glsl)

target_link_libraries(player_1 LINK_PUBLIC framework ${FFMPEG_LIBRARIES} ${FFMPEG_DEPENDENCIES}) 
target_link_libraries(player_2 LINK_PUBLIC framework ${FFMPEG_LIBRARIES} ${FFMPEG_DEPENDENCIES}) 
target_link_libraries(player_3 LINK_PUBLIC framework ${FFMPEG_LIBRARIES} ${FFMPEG_DEPENDENCIES} -lSDL) 
target_link_libraries(player_4 LINK_PUBLIC framework ${FFMPEG_LIBRARIES} ${FFMPEG_DEPENDENCIES} -lSDL) 
target_link_libraries(player_5 LINK_PUBLIC framework ${FFMPEG_LIBRARIES} ${FFMPEG_DEPENDENCIES}) 




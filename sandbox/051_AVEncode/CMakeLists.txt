add_executable (codec codec.cpp)
add_custom_command(TARGET codec POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/res)
target_link_libraries(codec LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} ${LIBAV_STATIC_LIBRARIES})

add_executable (filter_audio filter_audio.cpp)
target_link_libraries(filter_audio LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} ${LIBAV_STATIC_LIBRARIES})

add_executable (metadata metadata.cpp)
target_link_libraries(metadata LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} ${LIBAV_STATIC_LIBRARIES})

add_executable (output output.cpp)
target_link_libraries(output LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} ${LIBAV_STATIC_LIBRARIES})

add_executable (transcode_aac transcode_aac.cpp)
target_link_libraries(transcode_aac LINK_PUBLIC framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} ${LIBAV_STATIC_LIBRARIES})




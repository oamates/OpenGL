#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# framework static library
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
add_library (framework STATIC
    binpacker.cpp camera.cpp dds.cpp fbo.cpp font.cpp glfw_window.cpp glsl_noise.cpp image.cpp imgui_window.cpp hull3d.cpp log.cpp obj.cpp polyhedron.cpp 
    shader.cpp surface.cpp sphere.cpp tess.cpp torus.cpp uniform.cpp utils.cpp vbm.cpp vertex.cpp)

if(WIN32)
    set(ADDITIONAL_LIBRARIES opengl32)
else()
    set(ADDITIONAL_LIBRARIES "")
endif()

target_link_libraries(framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} ${PNG_STATIC_LIBRARIES} ${ZLIB_STATIC_LIBRARIES} ${PTHREAD} ${ADDITIONAL_LIBRARIES})
target_include_directories (framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# imgui static library
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
add_library (imgui STATIC imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp )
target_link_libraries(imgui framework)

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# video player static library :: built only if FFmpeg/LIBAV packages are found
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
if (FFMPEG_FOUND)
    add_library (video_player STATIC video/decoder.cpp)
    target_link_libraries(video_player framework ${FFMPEG_LIBRARIES} ${FFMPEG_DEPENDENCIES})
else (FFMPEG_FOUND)
    message("OpenGL video player static library will not be built ..")
endif (FFMPEG_FOUND)

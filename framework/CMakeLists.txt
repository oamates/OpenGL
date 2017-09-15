#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# framework static library
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
add_library (framework STATIC
    binpacker.cpp camera.cpp dds.cpp fbo.cpp font.cpp glfw_window.cpp glsl_noise.cpp image.cpp imgui_window.cpp hull3d.cpp log.cpp obj.cpp polyhedron.cpp 
    shader.cpp surface.cpp sphere.cpp tess.cpp torus.cpp uniform.cpp utils.cpp vbm.cpp vertex.cpp)

if(WIN32)
    set(ADDITIONAL_LIBRARIES opengl32)
else()
    set(ADDITIONAL_LIBRARIES pthread)
endif()

target_link_libraries(framework ${GLFW_STATIC_LIBRARIES} ${GLEW_STATIC_LIBRARIES} ${PNG_STATIC_LIBRARIES} ${ZLIB_STATIC_LIBRARIES} ${ADDITIONAL_LIBRARIES} image ${PTHREAD})
target_include_directories (framework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# image static library
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
add_library (image STATIC image/stb_image.cpp)
target_link_libraries(image framework)

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# oculus static library
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
add_library (ovr STATIC ovr/ovr_hmd.cpp)
target_include_directories (ovr PUBLIC ${PROJECT_SOURCE_DIR}/OculusVR/LibOVR/Include)
target_link_libraries(ovr framework LibOVR)

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# imgui static library
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
add_library (imgui STATIC imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp)
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

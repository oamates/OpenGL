# libavcodec libavformat libavutil libavdevice libavfilter libswresample libswscale
# FFMPEG_FOUND - system has ffmpeg or libav
# FFMPEG_INCLUDE_DIR - the ffmpeg include directory
# FFMPEG_LIBRARIES - Link these to use ffmpeg
# FFMPEG_LIBAVCODEC
# FFMPEG_LIBAVFORMAT
# FFMPEG_LIBAVUTIL
 

if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
    set(FFMPEG_FOUND TRUE)
else (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)

    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(_FFMPEG_AVCODEC libavcodec)
        pkg_check_modules(_FFMPEG_AVFORMAT libavformat)
        pkg_check_modules(_FFMPEG_AVUTIL libavutil)
        pkg_check_modules(_FFMPEG_AVDEVICE libavdevice)
        pkg_check_modules(_FFMPEG_AVFILTER libavfilter)
        pkg_check_modules(_FFMPEG_SWRESAMPLE libswresample)
        pkg_check_modules(_FFMPEG_SWSCALE libswscale)
    endif (PKG_CONFIG_FOUND)

    find_path(FFMPEG_AVCODEC_INCLUDE_DIR
        NAMES libavcodec/avcodec.h
        PATHS ${_FFMPEG_AVCODEC_INCLUDE_DIRS} /usr/include /usr/local/include
        PATH_SUFFIXES ffmpeg libav
    )

    find_library(FFMPEG_LIBAVCODEC
        NAMES avcodec
        PATHS ${_FFMPEG_AVCODEC_LIBRARY_DIRS} /usr/lib /usr/local/lib
    )

    find_library(FFMPEG_LIBAVFORMAT
        NAMES avformat
        PATHS ${_FFMPEG_AVFORMAT_LIBRARY_DIRS} /usr/lib /usr/local/lib
    )

    find_library(FFMPEG_LIBAVUTIL
        NAMES avutil
        PATHS ${_FFMPEG_AVUTIL_LIBRARY_DIRS} /usr/lib /usr/local/lib
    )

    find_library(FFMPEG_LIBAVDEVICE
        NAMES avdevice
        PATHS ${_FFMPEG_AVDEVICE_LIBRARY_DIRS} /usr/lib /usr/local/lib
    )

    find_library(FFMPEG_LIBAVFILTER
        NAMES avfilter
        PATHS ${_FFMPEG_AVFILTER_LIBRARY_DIRS} /usr/lib /usr/local/lib
    )

    find_library(FFMPEG_LIBSWRESAMPLE
        NAMES swresample
        PATHS ${_FFMPEG_SWRESAMPLE_LIBRARY_DIRS} /usr/lib /usr/local/lib
    )

    find_library(FFMPEG_LIBSWSCALE
        NAMES swscale
        PATHS ${_FFMPEG_SWSCALE_LIBRARY_DIRS} /usr/lib /usr/local/lib
    )

    if (FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT AND FFMPEG_LIBAVUTIL AND FFMPEG_LIBAVDEVICE AND FFMPEG_LIBAVFILTER AND FFMPEG_LIBSWRESAMPLE AND FFMPEG_LIBSWSCALE)
        set(FFMPEG_FOUND TRUE)
    endif()

    # linking order :: -lavformat -lavcodec -lswscale -lavutil -lswresample -lavfilter -lavdevice
    if (FFMPEG_FOUND)
        set(FFMPEG_INCLUDE_DIR ${FFMPEG_AVCODEC_INCLUDE_DIR})
        set(FFMPEG_LIBRARIES ${FFMPEG_LIBAVFORMAT} ${FFMPEG_LIBAVCODEC} ${FFMPEG_LIBSWSCALE} ${FFMPEG_LIBAVUTIL} ${FFMPEG_LIBSWRESAMPLE} ${FFMPEG_LIBAVFILTER} ${FFMPEG_LIBAVDEVICE})
    endif (FFMPEG_FOUND)

endif (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)



if (FFMPEG_FOUND)

    # Correct linking order on Linux :: 
    # -lavformat -lavcodec -lswscale -lavutil -lswresample -lavfilter -lavdevice -lfdk-aac -lmp3lame -lopus -ltheora -ltheoraenc -ltheoradec -lvorbis -lvorbisenc -lvpx -lx264 -lx265 -lva -lva-drm -lva-x11 -lvdpau -lz -lm)
    if(WIN32)
        set(FFMPEG_DEPENDENCIES "-lfdk-aac -lmp3lame -lopus -ltheora -ltheoraenc -ltheoradec -lvorbis -lvorbisenc -lvpx -lx264 -lx265 -lWs2_32 -liconv -lSecur32 -llzma -lbz2")
    else()
        set(FFMPEG_DEPENDENCIES "-lfdk-aac -lmp3lame -lopus -ltheora -ltheoraenc -ltheoradec -lvorbis -lvorbisenc -lvpx -lx264 -lx265 -lva -lva-drm -lva-x11 -lvdpau -lz -lm")
    endif()

    if (NOT FFMPEG_FIND_QUIETLY)
        message(STATUS "FFmpeg/Libav libraries    :: ${FFMPEG_LIBRARIES}")
        message(STATUS "FFmpeg/Libav include      :: ${FFMPEG_INCLUDE_DIR}")
        message(STATUS "FFmpeg/Libav dependencies :: ${FFMPEG_DEPENDENCIES}")
    endif (NOT FFMPEG_FIND_QUIETLY)

else (FFMPEG_FOUND)

    if (FFMPEG_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find libavcodec, libavformat, libavutil, libavdevice, libavfilter, libswresample or libswscale.")
    endif (FFMPEG_FIND_REQUIRED)

endif (FFMPEG_FOUND)

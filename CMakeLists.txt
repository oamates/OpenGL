#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Alexander Foksha :: OpenGL framework and demos project
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.1)
project (OpenGL)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# enable optimizations by setting one of the following compiler instruction set flags
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -msse2 -O3 -fabi-version=6")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -msse4 -O3 -fabi-version=6")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -mavx2 -mfma -O3 -fabi-version=6")

# If MINGW cannot find LTO plugin, do :: 
# mkdir mingw64\lib\bfd-plugins
# cp mingw64\lib\gcc\x86_64-w64-mingw32\{version number}\liblto_plugin-0.dll mingw64\lib\bfd-plugins\liblto_plugin-0.dll
# On Linux, do :: 
# mkdir usr/lib/bfd-plugins
# ln -s /usr/lib/gcc/x86_64-linux-gnu/5.4.0/liblto_plugin.so /usr/lib/bfd-plugins/



find_package(PkgConfig REQUIRED)

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# find and add GLFW, GLEW, PNG, Z and freetype packages
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
pkg_search_module(GLFW REQUIRED glfw3)
pkg_search_module(GLEW REQUIRED glew)
pkg_search_module(GLM REQUIRED glm)
pkg_search_module(PNG REQUIRED libpng)
pkg_search_module(ZLIB REQUIRED zlib)
pkg_search_module(FREETYPE REQUIRED freetype2)

include_directories(${GLFW_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${PNG_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# find optional FFmpeg package
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
find_package(FFmpeg REQUIRED)

# FFMPEG_FOUND - system has ffmpeg or libav
# FFMPEG_INCLUDE_DIR - the ffmpeg include directory
# FFMPEG_LIBRARIES 
# FFMPEG_LIBAVCODEC
# FFMPEG_LIBAVFORMAT
# FFMPEG_LIBAVUTIL

if (FFMPEG_FOUND)
    include_directories(${FFMPEG_INCLUDE_DIR})
else (FFMPEG_FOUND)
    message("Can not find FFmpeg/LIBAV. OpenGL video player library and some examples will not be built ..")
endif (FFMPEG_FOUND)


#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# common framework files, assembled into a static library
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
add_subdirectory (framework)
include_directories(framework)

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# opengl demos based on the framework
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
add_subdirectory (demos)

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# vulkan demos
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
if (NOT "$ENV{VULKAN_SDK}" STREQUAL "")
    set(VULKAN_INCLUDE_DIR "$ENV{VULKAN_SDK}/include")
    set(VULKAN_LIBRARY_PATH "$ENV{VULKAN_SDK}/lib")

    if(LINUX)
        set(VULKAN_LAYER_PATH "$ENV{VULKAN_SDK}/etc/explicit_layer.d")
    else()
        set(VULKAN_LAYER_PATH "")
    endif()

    message("Found Vulkan SDK installed at $ENV{VULKAN_SDK}")

	include_directories("${VULKAN_INCLUDE_DIR}")
	link_directories("${VULKAN_LIBRARY_PATH}") 
	link_directories("${VULKAN_LAYER_PATH}")
	add_subdirectory (vulkan)
else()
    message("Vulkan SDK is not installed or VULKAN_SDK environment variable not set")
    message("Skipping Vulkan demos build step")
endif()

#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# LibOVR and Oculus Rift demos
#------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
message("Path = ${PROJECT_SOURCE_DIR}/OculusVR")
set(OCULUS_LIBRARIES_PATH "${PROJECT_SOURCE_DIR}/OculusVR")

add_subdirectory ("${OCULUS_LIBRARIES_PATH}")
include_directories("${OCULUS_LIBRARIES_PATH}/LibOVR/Include")
add_subdirectory (demos_vr)

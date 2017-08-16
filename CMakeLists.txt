cmake_minimum_required(VERSION 3.7)
project(ffmpeg_tutorial)

set(CMAKE_C_STANDARD 11)

# include path
#include_directories(${PROJECT_SOURCE_DIR}/include/)

# link directories
link_directories(${PROJECT_SOURCE_DIR}/lib/)

link_libraries(swscale avdevice avformat avcodec avutil swresample postproc fdk-aac mp3lame opus x264 vpx pthread bz2 m z faac faad dl ass vdpau va X11 lzma vorbis vdpau va-drm va-x11)

set(SOURCE_FILES main.c)
add_executable(ffmpeg_tutorial ${SOURCE_FILES})
include_directories(${PROJECT_SOURCE_DIR}/include/)

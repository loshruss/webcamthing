cmake_minimum_required(VERSION 3.20)

project(webcamthing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavformat
    libavcodec
    libavutil
    libswscale
    libswresample
)

pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

add_executable(webcamthing
    src/main.cpp
    src/app.cpp
    src/media/player.cpp
    src/media/video_renderer.cpp
    src/media/audio_output.cpp
)

target_include_directories(webcamthing PRIVATE src)

target_link_libraries(webcamthing PRIVATE
    PkgConfig::FFMPEG
    PkgConfig::SDL2
)
cmake_minimum_required(VERSION 3.20)

project(webcamthing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(webcamthing
    src/main.cpp
    src/app.cpp
    src/media/player.cpp
)

target_include_directories(webcamthing PRIVATE
    src
)
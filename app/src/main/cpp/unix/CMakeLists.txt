cmake_minimum_required(VERSION 3.4.1)

project(ffsdk)
add_definitions(-Dunix)
add_definitions("-Wall -g")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src)
link_directories( ${CMAKE_SOURCE_DIR}/libs/)

message("libs folder : "${CMAKE_SOURCE_DIR}/libs)
message("include folder : "${CMAKE_SOURCE_DIR}/include)

find_package(PkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2)

message("libs sdl2 folder : "${SDL2_LIBRARIES})

# add_library(yuv STATIC yuv)
#add_library(Decoder src/InputFile.cpp src/Decoder.cpp src/Player.cpp src/VideoState.cpp)
add_executable(a main.cpp src/InputFile.cpp src/Decoder.cpp src/Player.cpp src/VideoState.cpp src/Clock.cpp)
target_link_libraries(a PUBLIC avformat avcodec avutil pthread swscale swresample jpeg avfilter yuv m z ${SDL2_LIBRARIES})

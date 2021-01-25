cmake_minimum_required(VERSION 3.4.1)

project(ffsdk)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories( ${CMAKE_SOURCE_DIR}/libs)

message("libs folder : "${CMAKE_SOURCE_DIR}/libs)

# add_library(yuv STATIC yuv)
add_library(InputFile InputFile.cpp)
add_library(Decoder Decoder.cpp)
add_executable(a main.cpp)
target_link_libraries(a PUBLIC InputFile Decoder jpeg avcodec avutil avformat avfilter swresample swscale yuv )

cmake_minimum_required(VERSION 3.4.1)

project(ffsdk)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/include/ ${CMAKE_SOURCE_DIR}/../src)
link_directories( ${CMAKE_SOURCE_DIR}/libs/)

message("libs folder : "${CMAKE_SOURCE_DIR}/libs)
message("include folder : "${CMAKE_SOURCE_DIR}/include)

# add_library(yuv STATIC yuv)
add_library(InputFile ../src/InputFile.cpp)
add_library(Decoder ../src/Decoder.cpp)
add_executable(a main.cpp)
target_link_libraries(a PUBLIC InputFile Decoder jpeg avcodec avutil avformat avfilter swresample swscale yuv )

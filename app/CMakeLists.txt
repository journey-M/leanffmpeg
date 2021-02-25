# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#SET(ASM_OPTIONS "-x assembler-with-cpp")
#SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )

add_definitions(-Dandroid)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


#设置安装的目录
set(distribution_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs)
include_directories(${CMAKE_SOURCE_DIR}/src/main/jniLibs/include)

# ffmpeg引用的library

add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libavcodec.so)
add_library(avfilter
        SHARED
        IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libavfilter.so)
add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libavformat.so)
add_library(avutil
        SHARED
        IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libavutil.so)
add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libswresample.so)
add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libswscale.so)
add_library(avdevice
        SHARED
        IMPORTED)
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libavdevice.so)
##添加yuv的库
add_library(yuv
        SHARED
        IMPORTED)
set_target_properties(yuv
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libyuv.so)

add_library(x264
        SHARED
        IMPORTED)
set_target_properties(x264
        PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/armeabi-v7a/libx264.so)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)
find_library(android android)
find_library(libc cutil)
find_library(jnigraphics jnigraphics)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.


##C: set(CMAKE_C_VISIBILITY_PRESET hidden)
##C++: set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_C_VISIBILITY_PRESET hidden)


#添加调试信息
add_definitions("-Wall -g")

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/jni/native-lib.cpp)

target_link_libraries( # Specifies the target library.
        native-lib
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        yuv
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${android})


add_library(
        push-lib
        SHARED
        src/main/cpp/jni/push-lib.cpp
)


target_link_libraries( # Specifies the target library.
        push-lib
        # Links the target library to the log library
        # included in the NDK.
        x264
        ${log-lib}
        ${android})

add_library(
        h264-lib
        SHARED
        src/main/cpp/jni/h264-lib.cpp
)

target_link_libraries( # Specifies the target library.
        h264-lib
        # Links the target library to the log library
        # included in the NDK.
        x264
        ${log-lib}
        ${android})


add_library(ffcmd
        SHARED
        src/main/cpp/jni/ffmpeg-invoke.cpp
        src/main/cpp/src_ffcmd/ffmpeg_opt.c
        src/main/cpp/src_ffcmd/ffmpeg.c
        src/main/cpp/src_ffcmd/ffmpeg_filter.c
        src/main/cpp/src_ffcmd/ffmpeg_hw.c
        src/main/cpp/src_ffcmd/cmdutils.c)

target_link_libraries( # Specifies the target library.
        ffcmd
        # Links the target library to the log library
        # included in the NDK.
        x264
        avcodec
        avfilter
        avdevice
        avformat
        avutil
        swresample
        swscale
        yuv
        ${log-lib}
        ${android})

add_library(
        videoEdit-lib
        SHARED
        src/main/cpp/jni/videoEdit-lib.cpp
        src/main/cpp/src/Decoder.cpp
        src/main/cpp/src/InputFile.cpp
        src/main/cpp/src/VideoState.cpp
        src/main/cpp/src/SafeVector.cpp
        src/main/cpp/src/Player.cpp
)

target_link_libraries( # Specifies the target library.
        videoEdit-lib
        # Links the target library to the log library
        # included in the NDK.
        jnigraphics
        x264
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        yuv
        ${log-lib}
        ${android})


add_library(
        vglplayer
        SHARED
        src/main/cpp/jni/glplayer/surface.cpp
        src/main/cpp/jni/glplayer/player.cpp
        src/main/cpp/jni/glplayer/util.cpp
        src/main/cpp/jni/glplayer/vdecode.cpp
        src/main/cpp/jni/glplayer/shader.cpp
)

target_link_libraries( # Specifies the target library.
        vglplayer
        # Links the target library to the log library
        # included in the NDK.
#        gl
        jnigraphics
        x264
        avcodec
        avfilter
        avformat
        avutil
        swresample
        swscale
        yuv
        EGL
        GLESv2
        ${log-lib}
        ${android})
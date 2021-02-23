//
// Created by gwj on 21-2-24.
//

#ifndef __FF_LOG_FF_H__
#define __FF_LOG_FF_H__

#ifdef android
#include <android/log.h>
#define FFlog(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__)
#elif unix
#define FFlog(FORMAT, ...) fprintf(stderr,FORMAT,##__VA_ARGS__)
#endif

#endif

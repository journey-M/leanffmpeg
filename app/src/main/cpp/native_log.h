#include <android/log.h>

#ifndef  __FFMPG_LOG__
#define  __FFMPG_LOG__
#define FFLOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);

#endif  __FFMPG_LOG__ /* __FFMPG_LOG__ */
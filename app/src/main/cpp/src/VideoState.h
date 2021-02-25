#ifndef _H__PLAYER_STATE__H_
#define _H__PLAYER_STATE__H_


#include "VideoState.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <time.h>
#include <sys/time.h>

}

class VideoState {
public:
    int video_seek_flag = 0;
    int video_seek_time = 0;

    VideoState();

    ~VideoState();
};


#endif

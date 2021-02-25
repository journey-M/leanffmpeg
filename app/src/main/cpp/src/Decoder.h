#ifndef _H_DECODER_H_
#define _H_DECODER_H_

#include "InputFile.h"
#include "VideoState.h"
#include "SafeVector.h"
#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>


extern "C" {
#include <libyuv.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#ifdef unix
#include <jpeglib.h>
#endif
}

using namespace std;

class Options {
public:
    long start;
    int per;
private:
};

struct FrameImage {
    int width;
    int height;
    char *buffer;
};

class Decoder {

public :
    InputFile *mInputFile;
    int videoStreamIndex = -1;
    AVStream *videoStream = NULL;
    int audioStreamIndex = -1;
    AVStream *audioStream = NULL;

    /**
     * 解码相关变量
     */
    AVCodec *videoCodec;
    AVCodec *audioCodec;
    AVCodecContext *vCodecCtx;
    AVCodecContext *aCodecCtx;

    /**
     * 缩略图相关变量
     */
    AVFrame *preFrame = NULL;
    int timedIndex = 0;
    double _time_val = (double )0.1;
    vector<string> v_path_results;


    /**
     *视频相关线程和栈
     */
    pthread_t th_read;
    pthread_t th_decode_video;
    pthread_t th_decode_audio;
    #define  MAX_AUDIO_PACKET_LIST_SIZE  20
    #define  MAX_VIDEO_PACKET_LIST_SIZE  30
    SafeVector<AVPacket *> videoPacketList;
    SafeVector<AVPacket *> audioPacketList;

    //多线程的锁
    pthread_cond_t *mutex_video_list_cond = PTHREAD_COND_INITIALIZER;
    pthread_cond_t mutex_audio_list_cond = PTHREAD_COND_INITIALIZER;

    //视频状态
    VideoState *videoState;


    Decoder(InputFile *inputFile);

    int getVideoImages(Options *option, vector<FrameImage *> *result, int maxcount);
    FrameImage *decodeOneFrame(int start, int cut);

    int writeJPEG(AVFrame *pFrame,const char *path,  int width, int height);

    vector<string> createVideoThumbs();

    void setPlayState(VideoState * state);

    void push_n_avframe(AVFrame * avFrame);

    void scalAndSaveFrame(AVFrame * avFrame,  double playTime);


    /**
     * 播放相关
     */
    void preperPlay();



private:


    int rgb2jpg(char *jpg_file, char *pdata, int width, int height);


    int findVideoStream();

    int findAudioStream();

    int initVideoDecoder();



    condition_variable video_cond;
    mutex video_pkg_list_mutex;
    condition_variable audio_cond;
    mutex audio_pkg_list_mutex;


};

#endif

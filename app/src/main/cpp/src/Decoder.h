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

//本地保存的
struct Frame{
    AVFrame *srcFrame;
    int width ;
    int height;
    int format;
    double pts;
    double duration;
    int64_t pos;
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
    SafeVector<AVPacket *> videoPacketList;
    SafeVector<AVPacket *> audioPacketList;
    #define  MAX_DISPLAY_FRAMS  16
    #define  MIN_DISPLAY_FRAMS  8
    vector<Frame*> display_list;

    //read线程锁
    pthread_mutex_t mutex_read_th = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond_read_th = PTHREAD_COND_INITIALIZER;

    //缓存Video Frame写入锁
    pthread_mutex_t mutex_video_frame_list = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond_video_frame_list = PTHREAD_COND_INITIALIZER;

    //缓存Audio Frame写入锁
    pthread_mutex_t mutex_audio_frame_list = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond_audio_frame_list = PTHREAD_COND_INITIALIZER;

    AVPacket flush_pkt;


    //视频状态
    VideoState *videoState = NULL;


    Decoder(InputFile *inputFile);

    int getVideoImages(Options *option, vector<FrameImage *> *result, int maxcount);
    FrameImage *decodeOneFrame(int start, int cut);

    int writeJPEG(AVFrame *pFrame,const char *path,  int width, int height);

    vector<string> createVideoThumbs();

    void setPlayState(VideoState * state);

    void push_n_avframe(AVFrame * avFrame);

    void scalAndSaveFrame(AVFrame * avFrame,  double playTime);

    int queue_picture(AVFrame *src_frame, double pts, double duration, int64_t pos);

    int pop_picture(Frame **frame);

    int queue_sample(AVFrame *src_frame, double pts, double duration, int64_t pos);

    int pop_sample(Frame **frame);


    /**
     * 播放相关
     */
    void preperPlay();

    Frame* getCurrentFrame();

    /**
     * 视频packet解码相关
     * @return
     */

    int decoder_decode_frame(AVFrame*, AVCodec* codec, AVCodecContext *codecCtx, SafeVector<AVPacket*>* queue);

    int get_video_frame(AVFrame* frame);


private:


    int rgb2jpg(char *jpg_file, char *pdata, int width, int height);


    int findVideoStream();

    int findAudioStream();

    int initVideoDecoder();

    int initAudioDecoder();



//    condition_variable video_cond;
//    mutex video_pkg_list_mutex;
//    condition_variable audio_cond;
//    mutex audio_pkg_list_mutex;


};

#endif

#ifndef _H_DECODER_H_
#define _H_DECODER_H_

#include "InputFile.h"
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
    //add for test
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
    #define  max_audio_packet_list_size  5
    #define  max_video_packet_list_size  20
     vector<AVPacket*> videoPacketList;
     vector<AVPacket*> audioPacketList;
     std::thread th_read_pkg;
     std::thread th_decode_video_pkg;
     std::thread th_decode_audio_pkg;



    Decoder(InputFile *inputFile);

    int getVideoImages(Options *option, vector<FrameImage *> *result, int maxcount);
    FrameImage *decodeOneFrame(int start, int cut);

    int writeJPEG(AVFrame *pFrame,const char *path,  int width, int height);

    vector<string> createVideoThumbs();


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

    //读包线程
    void th_read_packet();
    //
    void th_decode_video_packet();

    void th_decode_audio_packet();

};

#endif

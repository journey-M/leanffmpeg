#ifndef _H_DECODER_H_
#define _H_DECODER_H_

#include "InputFile.h"
#include <cstdint>
#include <vector>

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

    AVCodec *videoCodec;
    AVCodec *audioCodec;
    AVCodecContext *vCodecCtx;
    AVCodecContext *aCodecCtx;

    AVFrame *preFrame = NULL;
    int timedIndex = 0;
    double _time_val = (double )0.1;
    vector<string> v_path_results;

    Decoder(InputFile *inputFile);

    int getVideoImages(Options *option, vector<FrameImage *> *result, int maxcount);
    FrameImage *decodeOneFrame(int start, int cut);

    int writeJPEG(AVFrame *pFrame,const char *path,  int width, int height);

    vector<string> initVideoInfos();


    void push_n_avframe(AVFrame * avFrame);

    void scalAndSaveFrame(AVFrame * avFrame,  double playTime);


private:


    int rgb2jpg(char *jpg_file, char *pdata, int width, int height);


    int findVideoStream();

    int findAudioStream();

    int initVideoDecoder();

};

#endif

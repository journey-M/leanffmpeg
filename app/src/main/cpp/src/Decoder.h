#ifndef _H_DECODER_H_
#define _H_DECODER_H_

#include "InputFile.h"
#include <cstdint>
#include <vector>

extern "C" {
#include <libyuv.h>
#include <libavcodec/avcodec.h>
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

    Decoder(InputFile *inputFile);

    int getVideoImages(Options *option, vector<FrameImage *> *result, int maxcount);

private:

    void decodeVideo(InputFile *inputFile);

    void decodeAudio(InputFile *inputFile);

    int rgb2jpg(char *jpg_file, char *pdata, int width, int height);

    FrameImage *decodeOneFrame(int start);

    int findVideoStream();

    int findAudioStream();

    int initVideoDecoder();

};

#endif

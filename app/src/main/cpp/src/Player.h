#ifndef _H__PLAYER__H_
#define _H__PLAYER__H_


#include "InputFile.h"
#include "Decoder.h"
#include "VideoState.h"
#include <thread>
#include <map>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <time.h>
#include <sys/time.h>

}

struct Clock {
    double pts;
};


struct PlayCallback {
    int (*renderVideo)();

    int (*renderAudio)();
};

class Player {
public:
    PlayCallback *stCallback;

    vector<InputFile *> inputs_files;


    VideoState* state;


    Player();

    ~Player();

    void addInputFile(InputFile *inputFile);

    void removeInputFile(InputFile *inputFile);

    void setTimeStart(float start);

    int play();

    void playVideo();

    void playAudio();

    void setCallback(PlayCallback *scallback);

    void seekTimeLine();

    void preper();


private:
    map<InputFile*, Decoder*> decoder_maps;

    float time_start = 0.0f;

    void *readPacketTh(void *arg);

};


#endif

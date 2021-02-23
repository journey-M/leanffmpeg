#ifndef _H__PLAYER__H_
#define _H__PLAYER__H_


#include "InputFile.h"
#include "Decoder.h"
#include <thread>
#include <map>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <time.h>

}

struct Clock {
    double pts;
};

struct PlayerState {
    Clock videoClock;
    Clock audioClock;
};

struct PlayCallback {
    int (*renderVideo)();

    int (*renderAudio)();
};

class Player {
public:
    PlayCallback *stCallback;

    vector<InputFile *> inputs_files;

    Player();

    ~Player();

    void addInputFile(InputFile *inputFile);

    void removeInputFile(InputFile *inputFile);

    int play();

    void playVideo();

    void playAudio();

    void setCallback(PlayCallback *scallback);

    void seekTimeLine();


private:
    map<InputFile*, Decoder*> decoder_maps;

    void *readPacketTh(void *arg);

};


#endif

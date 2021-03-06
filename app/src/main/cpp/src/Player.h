#ifndef _H__PLAYER__H_
#define _H__PLAYER__H_


#include "Log.h"
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



struct PlayCallback {
    int (*renderVideo)();

    int (*renderAudio)();
};

class Player {
public:
    void (*display_back)(const AVFrame* frames) = NULL;
    void (* audio_callback)(unsigned char *, int size) = NULL;
    //视频的时钟
    Clock *audioClock;
    Clock *videoClock;
    double max_frame_duration = 10.0;



    map<InputFile*, Decoder*> decoder_maps;

    PlayCallback *stCallback;

    vector<InputFile *> inputs_files;


    VideoState* state;


    Player();

    ~Player();

    void addInputFile(InputFile *inputFile);

    void removeInputFile(InputFile *inputFile);

    void setTimeStart(float start);

    int play();


    void setCallback(PlayCallback *scallback);

    void seekTimeLine();

    void preper(void (*display_callback)(const AVFrame* frams), void (* audio_callback)(unsigned char *, int size));

    void getAudioBufferData(int* size, uint8_t *data);

    double vp_duration(double last_pts, DiaplayBufferFrame *nextvp);

    double compute_target_delay(double delay);




private:

    float time_start = 0.0f;

    void *readPacketTh(void *arg);

};


#endif

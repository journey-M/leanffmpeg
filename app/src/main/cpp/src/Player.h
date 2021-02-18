#ifndef _H__PLAYER__H_
#define _H__PLAYER__H_


#include "InputFile.h"
#include "Decoder.h"
#include <thread>
extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <time.h>

}

struct Clock{
  double pts;
};

struct PlayerState{
  Clock videoClock;
  Clock audioClock;
};

struct PlayCallback{
 int (*renderVideo)();
 int (*renderAudio)();
};

class Player{
  public:
    Decoder * decoder;
		AVFormatContext *fmt_ctx = NULL;
    PlayerState *state;
    PlayCallback *stCallback;
    vector<AVPacket*> videoPacketList;
    vector<AVPacket*> audioPacketList;


    Player(AVFormatContext *fmt_ctx, Decoder *dec);
    ~Player();

    int play(double start);

    void playVideo();
    void playAudio();

    void setCallback(PlayCallback *scallback);
    
    
  private:
    double start;
    static void* readPacketTh(void * arg);

    const int max_packet_list_size = 1024;
    //10毫秒
    const int max_packet_read_sleep_time = 10;

	int isPlaying = 0;
	pthread_t videoTh;
	pthread_cond_t videoThCond = PTHREAD_COND_INITIALIZER ;
	pthread_mutex_t videoMutex = PTHREAD_MUTEX_INITIALIZER;





};







#endif

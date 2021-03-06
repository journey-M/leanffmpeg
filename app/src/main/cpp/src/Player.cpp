#include "Player.h"

static pthread_t pid;
static int quit = 0;

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1



static void render_thread_start(void *args) {

    FFlog("render_thread_start ...");
    Player *player = static_cast<Player *>(args);

    while (!quit) {
        map<InputFile *, Decoder *>::iterator itor;
        itor = player->decoder_maps.begin();
        while (itor != player->decoder_maps.end()) {
            DisplayImage *displayImage = itor->second->getDisplayImage();
            if (displayImage != NULL) {
                player->videoClock->set_clock(displayImage->pts);
                //此处计算delay的时间

                double delay = 0;
                delay = player->compute_target_delay();
//                FFlog("video pts %lf,  audio pts %lf ,  delay ----- %lf \n ",
//                        player->videoClock->get_clock(),
//                        player->audioClock->get_clock(),delay);

                if (delay > 0) {
                    int  value = delay * 1000000;
                    FFlog("sleep ----- %d \n ", value);
                    av_usleep(value);
                }

                //显示到屏幕上
                if (player->display_back) {
                    player->display_back(displayImage->width, displayImage->height, displayImage->format, displayImage->buffer_size, &displayImage->dst_data[0][0]);
                }
                // delete []displayImage->buffer;
                av_freep(displayImage->dst_data);
                free(displayImage);
            }
            itor++;
        }
    }
}


Player::Player() {

    audioClock = new Clock();
    audioClock->init_clock();
    videoClock = new Clock();
    videoClock->init_clock();


    pthread_create(&pid, NULL, reinterpret_cast<void *(*)(void *)>(render_thread_start), this);

//    pthread_create(&pid, NULL, reinterpret_cast<void *(*)(void *)>(voice_player), this);

}

Player::~Player() {
}


void Player::addInputFile(InputFile *inputFile) {
    inputs_files.push_back(inputFile);
    decoder_maps.insert(pair<InputFile *, Decoder *>(inputFile, new Decoder(inputFile)));
}

void Player::removeInputFile(InputFile *inputFile) {
    for (vector<InputFile *>::iterator iter = inputs_files.begin();
         iter != inputs_files.end(); iter++) {
        if (*iter == inputFile) {

            //删除map里面的内容
            map<InputFile *, Decoder *>::iterator mapiter = decoder_maps.find(inputFile);
            delete mapiter->second;
            decoder_maps.erase(mapiter);

            //删除inputfiles里面的内容
            iter = inputs_files.erase(iter);
            iter--;
            //erase函数的返回指向当前被删除元素的下一个元素的迭代器
            return;
        }
    }
}

void Player::setCallback(PlayCallback *scallback) {
    stCallback = scallback;
}

void Player::setTimeStart(float start) {
//    this->time_start = start;
}

void Player::preper(void (*display_callback)(int width, int heigth ,int format, int buffer_size, const uint8_t *buffer),
                    void audio_callback(unsigned char *data, int size)) {
    this->display_back = display_callback;
    this->audio_callback = audio_callback;
    map<InputFile *, Decoder *>::iterator itor;
    itor = decoder_maps.begin();
    while (itor != decoder_maps.end()) {
        itor->second->preperPlay();
        itor->second->setDataPreperdCallback(reinterpret_cast<void (*)(void)>(audio_callback));
        itor++;
    }
}


int Player::play() {
//    if (isPlaying) {
//        return -1;
//    }
//    this->start = start;
//    int ret = pthread_create(&videoTh, NULL, (&Player::readPacketTh), this);
//    if (ret < 0) {
//        return -1;
//    }

    return 0;
}

void Player::getAudioBufferData(int *size, uint8_t *data) {
    map<InputFile *, Decoder *>::iterator itor;
    itor = decoder_maps.begin();
    while (itor != decoder_maps.end()) {
        //播放音频数据
        AudioBufferFrame *aBuffer = itor->second->getAudioFrame();
        if (aBuffer != NULL) {
            *size = aBuffer->size;
            if(*size > 0){
                memcpy(data, aBuffer->buffer, aBuffer->size);
            }
            free(aBuffer->buffer);
            free(aBuffer);
            //设置时间时钟
            audioClock->set_clock(aBuffer->pts);
            return;
        }
        itor++;
    }

    *size = 0;
}

double Player::vp_duration(double last_video_pts, DiaplayBufferFrame *nextvp) {
    double duration = nextvp->pts - last_video_pts;
    if (isnan(duration) || duration <= 0 || duration > max_frame_duration)
        return duration;
    else
        return duration;
}


double Player::compute_target_delay() {
    double sync_threshold, diff = 0;

    //非主时钟
    /* update delay to follow master synchronisation source */

    /* if video is slave, we try to correct big delays by
       duplicating or deleting a frame */
    diff = videoClock->get_clock() - audioClock->get_clock();

    /* skip or repeat frame. We take into account the
       delay to compute the threshold. I still don't know
       if it is the best guess */
//    sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
//    if (!isnan(diff) && fabs(diff) < max_frame_duration) {
//        if (diff <= -sync_threshold)
//            delay = FFMAX(0, delay + diff);
//        else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
//            delay = delay + diff;
//        else if (diff >= sync_threshold)
//            delay = 2 * delay;
//    }

//    FFlog( "video: delay=%0.3f A-V=%f\n",delay, -diff);

    return diff;
}



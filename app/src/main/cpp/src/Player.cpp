#include "Player.h"


Player::Player(AVFormatContext *fmt_c, Decoder *dec) {
    this->decoder = dec;
    this->fmt_ctx = fmt_c;
    state = new PlayerState();
}

Player::~Player() {
    delete state;
}


void *Player::readPacketTh(void *playPtr) {
    Player *player;
    if (player) {
        player = (Player *) playPtr;
    }

    int ret = -1;
    AVRational time_base = player->decoder->videoStream->time_base;
    int64_t seekPos = player->start / av_q2d(time_base);

    ret = avformat_seek_file(player->fmt_ctx, player->decoder->videoStreamIndex, INT64_MIN, seekPos,
                             seekPos,
                             AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    if (ret < 0) {
        fprintf(stderr, "seek faild \n");
        return NULL;
    }

    for (;;) {
        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(player->fmt_ctx, packet);
        if (ret < 0) {
            //文件结尾或者 出错
            av_packet_free(&packet);
            break;
        } else if (ret >= 0) {

            //判断当前解码的packet栈是否满了
            if (player->audioPacketList.size() + player->audioPacketList.size() >
                player->max_packet_list_size) {
                //set the max time to wait
                struct timeval now;
                struct timespec timeSpec;
                gettimeofday(&now, NULL);
                timeSpec.tv_sec = now.tv_sec;
                timeSpec.tv_nsec =
                        now.tv_usec * 1000 + player->max_packet_read_sleep_time * 1000000;
                pthread_mutex_lock(&player->videoMutex);
                pthread_cond_timedwait(&player->videoThCond, &player->videoMutex, &timeSpec);
                pthread_mutex_unlock(&player->videoMutex);
            }

            if (player->decoder->videoStreamIndex > 0 &&
                packet->stream_index == player->decoder->videoStreamIndex) {
                player->videoPacketList.push_back(packet);
            } else if (player->decoder->audioStreamIndex > 0 &&
                       packet->stream_index == player->decoder->videoStreamIndex) {
                player->audioPacketList.push_back(packet);
            }
        }
    }

    return NULL;
}

void Player::setCallback(PlayCallback *scallback) {
    stCallback = scallback;
}


int Player::play(double start) {
    if (isPlaying) {
        return -1;
    }
    this->start = start;
    int ret = pthread_create(&videoTh, NULL, (&Player::readPacketTh), this);
    if (ret < 0) {
        return -1;
    }

    return 0;
}

void Player::playAudio() {


}

void Player::playVideo() {


}

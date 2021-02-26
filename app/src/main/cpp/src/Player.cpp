#include "Player.h"

static pthread_t  pid;
static int quit = 0;
static void render_thread_start(void *args){

    FFlog("render_thread_start ...");
    Player* player = static_cast<Player*>(args);

    while (!quit){
        //休眠1秒
        std::this_thread::sleep_for(std::chrono::seconds(1));
        map<InputFile*,Decoder*> ::iterator itor;
        itor = player->decoder_maps.begin();
        while (itor != player->decoder_maps.end()){
            Frame *frame = itor->second->getCurrentFrame();
            if(frame){
                //显示到屏幕上
                if(player->call_back){
                    player->call_back(frame->srcFrame);
                }

            }
            itor++;
        }
    }
}


Player::Player() {
    pthread_create(&pid,NULL,reinterpret_cast<void *(*)(void *)>(render_thread_start) ,this);
}

Player::~Player() {
}


void Player::addInputFile(InputFile *inputFile) {
    inputs_files.push_back(inputFile);
    decoder_maps.insert(pair<InputFile*, Decoder*>(inputFile, new Decoder(inputFile)));
}

void Player::removeInputFile(InputFile *inputFile) {
    for (vector<InputFile *>::iterator iter = inputs_files.begin();
         iter != inputs_files.end(); iter++) {
        if (*iter == inputFile) {

            //删除map里面的内容
            map<InputFile*, Decoder*>::iterator mapiter = decoder_maps.find(inputFile);
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

void Player:: setTimeStart(float start){
    this->time_start = start;
}

void Player::preper(void (*call_b)(AVFrame* frams)){
    this->call_back = call_b;
    map<InputFile*,Decoder*> ::iterator itor;
    itor = decoder_maps.begin();
    while (itor != decoder_maps.end()){
        itor->second->preperPlay();
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

void Player::playAudio() {


}

void Player::playVideo() {


}

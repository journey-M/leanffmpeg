#include "Player.h"


Player::Player() {
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

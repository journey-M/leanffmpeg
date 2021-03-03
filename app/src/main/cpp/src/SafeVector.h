#ifndef _H__SAFE_VECTOR__H_
#define _H__SAFE_VECTOR__H_

#include "InputFile.h"
#include "VideoState.h"
#include "Log.h"
#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>


extern "C" {
#include <libyuv.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
template <typename T>
//typedef T AVPacket* ;
class SafeVector {

private:
    std::vector<T> *elems;     // 元素
    char* TAG = "test";
    pthread_mutex_t mutex_list = PTHREAD_MUTEX_INITIALIZER;


public:

    int packet_pending = 0 ;


    SafeVector();

    ~SafeVector();

    void push_value(T t);

    T pop_value();

    int getSize();

    int enough();

    int not_enough();

    int isDecodeing();

    void setTag(char* tag);

};

template<typename T> SafeVector<T>::SafeVector() {
    elems = new std::vector<T>(30);
}

template<typename T> SafeVector<T>::~SafeVector() {

}

template<typename T> void SafeVector<T>::push_value(T t) {
    pthread_mutex_lock(&mutex_list);
    elems->push_back(t);
    pthread_mutex_unlock(&mutex_list);
}

template<typename T> T SafeVector<T>::pop_value() {
    T t;
    if (elems->size() == 0){
        return NULL;
    }
    pthread_mutex_lock(&mutex_list);
    t = elems->front();
    elems->erase(elems->begin());
    pthread_mutex_unlock(&mutex_list);
    return t;
}


template<typename T> int SafeVector<T>::getSize(){
    int size = elems->size();
    // FFlog("%s size = %d \n", TAG, size);
    return size;
}

template<typename T> int SafeVector<T>::enough(){
    return getSize() > 20;
}


template<typename T> int SafeVector<T>::not_enough(){
    return getSize() < 10;
}

template<typename T> int SafeVector<T>::isDecodeing(){
    return 0;
}

template<typename T> void SafeVector<T>::setTag(char* tag){
    TAG = tag;
}


#endif

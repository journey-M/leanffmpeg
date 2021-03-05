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

    int max_number;
    int min_number;

public:


    SafeVector(int min, int max);

    ~SafeVector();

    void push_value(T t);

    T pop_value();

    int getSize();

    int enough();

    int not_enough();

    int isDecodeing();

    void setTag(char* tag);

    void printSize();

};

template<typename T> SafeVector<T>::SafeVector(int min, int max) {
    elems = new std::vector<T>();
    this->min_number = min;
    this->max_number = max;
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
    return size;
}

template<typename T> int SafeVector<T>::enough(){
    return getSize() > max_number;
}


template<typename T> int SafeVector<T>::not_enough(){
    return getSize() < min_number;
}

template<typename T> int SafeVector<T>::isDecodeing(){
    return 0;
}

template<typename T> void SafeVector<T>::setTag(char* tag){
    TAG = tag;
}

template<typename T> void SafeVector<T>::printSize(){
//    FFlog("%s  size is : %d  \n" , TAG, getSize());
}



#endif

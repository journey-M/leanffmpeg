#ifndef _H__SAFE_VECTOR__H_
#define _H__SAFE_VECTOR__H_

#include "InputFile.h"
#include "VideoState.h"
#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>


extern "C" {
#include <libyuv.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
template <class T>
class SafeVector {

private:
    std::vector<T> elems;     // 元素
    pthread_mutex_t mutex_list = PTHREAD_MUTEX_INITIALIZER;


public:

    int packet_pending = 0 ;


    SafeVector();

    ~SafeVector();

    void push_value(T t);

    T pop_value();

    int getSize();

    int enough();

    int isDecodeing();

};

template<class T> SafeVector<T>::SafeVector() {

}

template<class T> SafeVector<T>::~SafeVector() {

}

template<class T> void SafeVector<T>::push_value(T t) {
    pthread_mutex_lock(&mutex_list);
    elems.push_back(t);
    pthread_mutex_unlock(&mutex_list);
}

template<class T> T SafeVector<T>::pop_value() {
    T t;
    if (elems.size() == 0){
        return NULL;
    }
    pthread_mutex_lock(&mutex_list);
    t = elems.front();
    pthread_mutex_unlock(&mutex_list);
    return t;
}


template<class T> int SafeVector<T>::getSize(){
    return elems.size();
}

template<class T> int SafeVector<T>::enough(){
    return elems.size() > 20;
}


template<class T> int SafeVector<T>::isDecodeing(){
    return 0;
}


#endif
